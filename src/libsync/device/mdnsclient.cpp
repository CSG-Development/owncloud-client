#include "mdnsclient.h"

#include <algorithm>
#include <QLoggingCategory>
#include <QNetworkInterface>
#include <QStringList>

Q_LOGGING_CATEGORY(lcMdnsDevice, "mdns.device", QtDebugMsg)

namespace {

constexpr int MinRecordTtlMs = 2 * 1000;
constexpr int MaxRecordTtlMs = 60 * 1000;
constexpr int ScanIntervalMs = 1 * 1000;
constexpr int DebounceMs = ScanIntervalMs / 2;
constexpr int InitialWaitMs = 2500;
constexpr quint16 MdnsPort = 5353;

const auto ServiceTypeHttps = QByteArrayLiteral("_https._tcp.local.");
const auto TxtKeySeagate = QByteArrayLiteral("seagate");
const auto TxtValueHomecloud = QByteArrayLiteral("homecloud");
const auto MdnsGroupAddress = QHostAddress(QStringLiteral("224.0.0.251"));

QByteArray buildQueryPacket(bool preferUnicastResponse)
{
    QByteArray packet;
    packet.reserve(30);

    packet.append('\x00');
    packet.append('\x00');
    packet.append('\x00');
    packet.append('\x00');
    packet.append('\x00');
    packet.append('\x01');
    packet.append('\x00');
    packet.append('\x00');
    packet.append('\x00');
    packet.append('\x00');
    packet.append('\x00');
    packet.append('\x00');
    packet.append('\x06');
    packet.append("_https", 6);
    packet.append('\x04');
    packet.append("_tcp", 4);
    packet.append('\x05');
    packet.append("local", 5);
    packet.append('\x00');
    packet.append('\x00');
    packet.append(static_cast<char>(MDNS_RECORDTYPE_PTR));
    packet.append(preferUnicastResponse ? '\x80' : '\x00');
    packet.append(static_cast<char>(MDNS_CLASS_IN));
    return packet;
}

template<class T>
bool parseInt(const QByteArray& packet, quint16& offset, T& value)
{
    if (offset + sizeof(T) > static_cast<unsigned int>(packet.length())) {
        return false;
    }

    value = qFromBigEndian<T>(reinterpret_cast<const uchar*>(packet.constData() + offset));
    offset += sizeof(T);
    return true;
}

bool parseName(const QByteArray& packet, quint16& offset, QByteArray& name)
{
    quint16 offsetEnd = 0;
    quint16 offsetPtr = offset;

    forever {
        quint8 bytesCount = 0;
        if (!parseInt<quint8>(packet, offset, bytesCount)) {
            return false;
        }

        if (!bytesCount) {
            break;
        }

        switch (bytesCount & 0xc0) {
        case 0x00:
            if (offset + bytesCount > packet.length()) {
                return false;
            }

            name.append(packet.mid(offset, bytesCount));
            name.append('.');
            offset += bytesCount;
            break;

        case 0xc0: {
            quint8 bytesCount2 = 0;
            quint16 newOffset = 0;
            if (!parseInt<quint8>(packet, offset, bytesCount2)) {
                return false;
            }

            newOffset = ((bytesCount & ~0xc0) << 8) | bytesCount2;
            if (newOffset >= offsetPtr) {
                return false;
            }

            offsetPtr = newOffset;
            if (!offsetEnd) {
                offsetEnd = offset;
            }

            offset = newOffset;
            break;
        }

        default:
            return false;
        }
    }

    if (offsetEnd) {
        offset = offsetEnd;
    }

    return true;
}

bool parseRecord(const QByteArray& packet, quint16& offset, DnsRecord& record)
{
    QByteArray name;
    quint16 type = 0;
    quint16 class_ = 0;
    quint16 dataLen = 0;
    quint32 ttl = 0;

    if (!parseName(packet, offset, name)
        || !parseInt<quint16>(packet, offset, type)
        || !parseInt<quint16>(packet, offset, class_)
        || !parseInt<quint32>(packet, offset, ttl)
        || !parseInt<quint16>(packet, offset, dataLen)) {
        return false;
    }

    record.name = name;
    record.type = type;
    record.flushCache = class_ & 0x8000;
    record.ttl = ttl;

    switch (type) {
    case MDNS_RECORDTYPE_A: {
        quint32 ipv4Addr = 0;
        if (!parseInt<quint32>(packet, offset, ipv4Addr)) {
            return false;
        }

        record.address = QHostAddress(ipv4Addr);
        break;
    }
    case MDNS_RECORDTYPE_AAAA:
        if (offset + 16 > packet.length()) {
            return false;
        }
        offset += 16;
        break;
    case MDNS_RECORDTYPE_NSEC: {
        QByteArray nextDomainName;
        quint8 number = 0;
        quint8 length = 0;
        if (!parseName(packet, offset, nextDomainName)
            || !parseInt<quint8>(packet, offset, number)
            || !parseInt<quint8>(packet, offset, length)
            || number != 0
            || offset + length > packet.length()) {
            return false;
        }

        offset += length;
        break;
    }
    case MDNS_RECORDTYPE_PTR: {
        QByteArray target;
        if (!parseName(packet, offset, target)) {
            return false;
        }

        record.target = target;
        break;
    }
    case MDNS_RECORDTYPE_SRV: {
        quint16 priority = 0;
        quint16 weight = 0;
        quint16 port = 0;
        QByteArray target;
        if (!parseInt<quint16>(packet, offset, priority)
            || !parseInt<quint16>(packet, offset, weight)
            || !parseInt<quint16>(packet, offset, port)
            || !parseName(packet, offset, target)) {
            return false;
        }

        record.priority = priority;
        record.weight = weight;
        record.port = port;
        record.target = target;
        break;
    }
    case MDNS_RECORDTYPE_TXT: {
        const auto start = offset;
        while (offset < start + dataLen) {
            quint8 bytes = 0;
            if (!parseInt<quint8>(packet, offset, bytes) || offset + bytes > packet.length()) {
                return false;
            }

            if (bytes == 0) {
                break;
            }

            const QByteArray attr(packet.constData() + offset, bytes);
            offset += bytes;
            const auto splitIndex = attr.indexOf('=');
            if (splitIndex == -1) {
                record.attributes.insert(attr, QByteArray());
            } else {
                record.attributes.insert(attr.left(splitIndex), attr.mid(splitIndex + 1));
            }
        }
        break;
    }
    default:
        offset += dataLen;
        break;
    }

    return true;
}

bool fromPacket(const QByteArray& packet, Message& message)
{
    quint16 offset = 0;
    quint16 transactionId = 0;
    quint16 flags = 0;
    quint16 questionCount = 0;
    quint16 answerCount = 0;
    quint16 authorityCount = 0;
    quint16 additionalCount = 0;
    if (!parseInt<quint16>(packet, offset, transactionId)
        || !parseInt<quint16>(packet, offset, flags)
        || !parseInt<quint16>(packet, offset, questionCount)
        || !parseInt<quint16>(packet, offset, answerCount)
        || !parseInt<quint16>(packet, offset, authorityCount)
        || !parseInt<quint16>(packet, offset, additionalCount)) {
        return false;
    }

    for (quint16 i = 0; i < questionCount; ++i) {
        QByteArray questionName;
        quint16 questionType = 0;
        quint16 questionClass = 0;
        if (!parseName(packet, offset, questionName)
            || !parseInt<quint16>(packet, offset, questionType)
            || !parseInt<quint16>(packet, offset, questionClass)) {
            return false;
        }
    }

    message.transactionId = transactionId;
    message.isResponse = flags & 0x8400;
    message.isTruncated = flags & 0x0200;

    const auto recordCount = static_cast<quint16>(answerCount + authorityCount + additionalCount);
    for (quint16 i = 0; i < recordCount; ++i) {
        DnsRecord record;
        if (!parseRecord(packet, offset, record)) {
            return false;
        }

        message.records.append(record);
    }

    return true;
}

int ttlToMs(quint32 ttlSeconds)
{
    if (ttlSeconds == 0) {
        return MinRecordTtlMs;
    }

    const auto ttlMs = static_cast<int>(ttlSeconds * 1000);
    return std::clamp(ttlMs, MinRecordTtlMs, MaxRecordTtlMs);
}

void appendUniqueAddress(QList<QHostAddress>& addresses, const QHostAddress& address)
{
    if (address.isNull()) {
        return;
    }

    if (std::find(addresses.cbegin(), addresses.cend(), address) == addresses.cend()) {
        addresses.append(address);
    }
}

bool isUsableIpv4Entry(const QNetworkInterface& iface, const QNetworkAddressEntry& entry)
{
    if (!iface.isValid() || !iface.flags().testFlag(QNetworkInterface::IsUp)
        || !iface.flags().testFlag(QNetworkInterface::IsRunning)
        || iface.flags().testFlag(QNetworkInterface::IsLoopBack)
        || entry.ip().protocol() != QAbstractSocket::IPv4Protocol
        || entry.ip().isNull()
        || entry.netmask().isNull()) {
        return false;
    }

    return true;
}

bool isHttpsServiceInstanceName(const QByteArray& name)
{
    return name.endsWith(QByteArrayLiteral("._https._tcp.local."));
}

} // namespace

MdnsClient::MdnsClient(QObject* parent)
    : QObject(parent)
{
    setupSockets();

    scanTimer_.setInterval(ScanIntervalMs);
    connect(&scanTimer_, &QTimer::timeout, this, &MdnsClient::performScanCycle);

    debounceTimer_.setInterval(DebounceMs);
    debounceTimer_.setSingleShot(true);

    notFoundTimer_.setInterval(InitialWaitMs);
    notFoundTimer_.setSingleShot(true);

    connect(&notFoundTimer_, &QTimer::timeout, this, [this] {
        debounceTimer_.start();
    });

    connect(&debounceTimer_, &QTimer::timeout, this, [this] {
        notFoundTimer_.stop();
        logDiscoverySummary(lastData_.isEmpty() ? QStringLiteral("complete-empty") : QStringLiteral("complete"));
        emit resultsChanged(lastData_);
    });

    connect(this, &MdnsClient::resultsChanged_internal, this, [this](const QList<DevicePath>& records) {
        lastData_ = records;
        debounceTimer_.start();
    });
}

MdnsClient::~MdnsClient()
{
    stop();

    if (multicastSocket_) {
        multicastSocket_->abort();
    }
    if (unicastSocket_) {
        unicastSocket_->abort();
    }
}

void MdnsClient::setupSockets()
{
    multicastSocket_ = new QUdpSocket(this);
    if (multicastSocket_->bind(QHostAddress::AnyIPv4, MdnsPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        connect(multicastSocket_, &QUdpSocket::readyRead, this, &MdnsClient::onReadyRead);
        joinMulticastGroups();
    } else {
        qCDebug(lcMdnsDevice) << "Failed bind multicast socket";
        multicastSocket_->deleteLater();
        multicastSocket_ = nullptr;
    }

    unicastSocket_ = new QUdpSocket(this);
    if (unicastSocket_->bind(QHostAddress::AnyIPv4, 0, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        connect(unicastSocket_, &QUdpSocket::readyRead, this, &MdnsClient::onReadyRead);
        qCDebug(lcMdnsDevice) << "Socket binded to" << unicastSocket_->localAddress() << unicastSocket_->localPort();
    } else {
        qCDebug(lcMdnsDevice) << "Failed bind unicast socket";
        unicastSocket_->deleteLater();
        unicastSocket_ = nullptr;
    }
}

void MdnsClient::joinMulticastGroups()
{
    if (!multicastSocket_) {
        return;
    }

    for (const auto& iface : QNetworkInterface::allInterfaces()) {
        if (!iface.isValid() || !iface.flags().testFlag(QNetworkInterface::IsUp)
            || !iface.flags().testFlag(QNetworkInterface::IsRunning)
            || iface.flags().testFlag(QNetworkInterface::IsLoopBack)
            || !iface.flags().testFlag(QNetworkInterface::CanMulticast)) {
            continue;
        }

        if (joinedMulticastIfaces_.contains(iface.name())) {
            continue;
        }

        if (multicastSocket_->joinMulticastGroup(MdnsGroupAddress, iface)) {
            joinedMulticastIfaces_.insert(iface.name());
            qCDebug(lcMdnsDevice) << "Joined mDNS multicast group on" << iface.humanReadableName();
        } else {
            qCDebug(lcMdnsDevice) << "Failed to join mDNS multicast group on" << iface.humanReadableName();
        }
    }
}

void MdnsClient::start()
{
    stop();
    resetDiagnostics();

    joinMulticastGroups();

    services_.clear();
    hosts_.clear();
    discoveredRecords_.clear();
    lastSeen_.clear();
    recordTtlMs_.clear();
    lastData_.clear();

    notFoundTimer_.start();

    performScanCycle();
    scanTimer_.start();
}

void MdnsClient::resetDiagnostics()
{
    diagnostics_ = {};
}

void MdnsClient::logDiscoverySummary(const QString& phase) const
{
    QStringList parts;
    parts.append(QStringLiteral("phase:%1").arg(phase));
    parts.append(QStringLiteral("results:%1").arg(discoveredRecords_.size()));
    parts.append(QStringLiteral("mdnsPackets:%1").arg(diagnostics_.mdnsPackets));
    parts.append(QStringLiteral("mdnsRecords:%1").arg(diagnostics_.mdnsRecords));
    parts.append(QStringLiteral("parseFailures:%1").arg(diagnostics_.parseFailures));
    parts.append(QStringLiteral("httpsPtrs:%1").arg(diagnostics_.mdnsHttpsPtrRecords));
    parts.append(QStringLiteral("allCandidates:%1").arg(diagnostics_.allCandidateEndpoints.size()));
    parts.append(QStringLiteral("mdnsCandidates:%1").arg(diagnostics_.mdnsServiceCandidates));
    parts.append(QStringLiteral("notHomecloudMdnsCandidates:%1").arg(diagnostics_.notHomecloudMdnsCandidates));

    qCInfo(lcMdnsDevice).noquote()
        << "Discovery summary"
        << QStringLiteral("{%1}").arg(parts.join(QStringLiteral(",")));
}

void MdnsClient::logAllCandidateEndpoint(const QString& address, quint16 port, const QString& source)
{
    const auto key = QStringLiteral("%1:%2").arg(address, QString::number(port));
    if (diagnostics_.allCandidateEndpoints.contains(key)) {
        return;
    }

    diagnostics_.allCandidateEndpoints.insert(key);

    QStringList parts;
    parts.append(QStringLiteral("endpoint:%1").arg(key));
    parts.append(QStringLiteral("source:%1").arg(source));

    qCInfo(lcMdnsDevice).noquote()
        << "Discovery candidate"
        << QStringLiteral("{%1}").arg(parts.join(QStringLiteral(",")));
}

void MdnsClient::logAcceptedEndpoint(const QString& address, quint16 port, const QString& source)
{
    const auto key = QStringLiteral("%1:%2").arg(address, QString::number(port));
    if (diagnostics_.acceptedEndpoints.contains(key)) {
        return;
    }

    diagnostics_.acceptedEndpoints.insert(key);

    QStringList parts;
    parts.append(QStringLiteral("endpoint:%1").arg(key));
    parts.append(QStringLiteral("source:%1").arg(source));

    qCInfo(lcMdnsDevice).noquote()
        << "Discovery accepted"
        << QStringLiteral("{%1}").arg(parts.join(QStringLiteral(",")));
}

void MdnsClient::logNotHomecloudEndpoint(const QString& address, quint16 port, const QString& source)
{
    const auto key = QStringLiteral("%1:%2").arg(address, QString::number(port));
    if (diagnostics_.notHomecloudCandidateEndpoints.contains(key)) {
        return;
    }

    diagnostics_.notHomecloudCandidateEndpoints.insert(key);
    ++diagnostics_.notHomecloudMdnsCandidates;

    QStringList parts;
    parts.append(QStringLiteral("endpoint:%1").arg(key));
    parts.append(QStringLiteral("source:%1").arg(source));

    qCInfo(lcMdnsDevice).noquote()
        << "Discovery not-homecloud"
        << QStringLiteral("{%1}").arg(parts.join(QStringLiteral(",")));
}

void MdnsClient::stop()
{
    scanTimer_.stop();
    debounceTimer_.stop();
    notFoundTimer_.stop();
}

void MdnsClient::performScanCycle()
{
    if (pruneExpiredState()) {
        emit resultsChanged_internal(discoveredRecords_.values());
    }

    const auto multicastQuery = buildQueryPacket(false);
    const auto unicastQuery = buildQueryPacket(true);
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const auto& iface : interfaces) {
        bool hasUsableIpv4 = false;
        for (const auto& entry : iface.addressEntries()) {
            if (isUsableIpv4Entry(iface, entry)) {
                hasUsableIpv4 = true;
                break;
            }
        }
        if (!hasUsableIpv4) {
            continue;
        }

        if (multicastSocket_ && iface.flags().testFlag(QNetworkInterface::CanMulticast)) {
            multicastSocket_->setMulticastInterface(iface);
            multicastSocket_->writeDatagram(multicastQuery, MdnsGroupAddress, MdnsPort);
        }

        if (unicastSocket_) {
            unicastSocket_->setMulticastInterface(iface);
            unicastSocket_->writeDatagram(unicastQuery, MdnsGroupAddress, MdnsPort);
        }
    }
}

bool MdnsClient::pruneExpiredState()
{
    const auto now = QDateTime::currentDateTimeUtc();
    bool changed = false;

    for (auto it = lastSeen_.begin(); it != lastSeen_.end();) {
        const auto ttlMs = recordTtlMs_.value(it.key(), MinRecordTtlMs);
        if (it.value().msecsTo(now) > ttlMs) {
            discoveredRecords_.remove(it.key());
            recordTtlMs_.remove(it.key());
            it = lastSeen_.erase(it);
            changed = true;
        } else {
            ++it;
        }
    }

    for (auto it = services_.begin(); it != services_.end();) {
        if (it->expiresAt.isValid() && it->expiresAt <= now) {
            it = services_.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = hosts_.begin(); it != hosts_.end();) {
        if (it->expiresAt.isValid() && it->expiresAt <= now) {
            it = hosts_.erase(it);
        } else {
            ++it;
        }
    }

    return changed;
}

void MdnsClient::onReadyRead()
{
    auto* socket = qobject_cast<QUdpSocket*>(sender());
    if (!socket) {
        return;
    }

    while (socket->hasPendingDatagrams()) {
        QByteArray packet;
        packet.resize(socket->pendingDatagramSize());

        QHostAddress senderAddress;
        quint16 senderPort = 0;
        socket->readDatagram(packet.data(), packet.size(), &senderAddress, &senderPort);

        Message message;
        if (!fromPacket(packet, message)) {
            ++diagnostics_.parseFailures;
            continue;
        }

        ++diagnostics_.mdnsPackets;
        diagnostics_.mdnsRecords += message.records.size();
        processMessage(message, senderAddress);
    }
}

void MdnsClient::processMessage(const Message& message, const QHostAddress& senderAddress)
{
    const auto now = QDateTime::currentDateTimeUtc();
    QSet<QByteArray> touchedInstances;

    for (const auto& record : message.records) {
        const auto expiresAt = now.addMSecs(ttlToMs(record.ttl));
        switch (record.type) {
        case MDNS_RECORDTYPE_PTR:
            if (record.name == ServiceTypeHttps && !record.target.isEmpty()) {
                ++diagnostics_.mdnsHttpsPtrRecords;
                auto& state = services_[record.target];
                state.serviceType = record.name;
                state.expiresAt = expiresAt;
                state.ttlMs = ttlToMs(record.ttl);
                appendUniqueAddress(state.sourceAddresses, senderAddress);
                touchedInstances.insert(record.target);
            }
            break;

        case MDNS_RECORDTYPE_SRV: {
            if (!isHttpsServiceInstanceName(record.name)) {
                break;
            }
            auto& state = services_[record.name];
            if (state.serviceType.isEmpty()) {
                state.serviceType = ServiceTypeHttps;
            }
            state.hostName = record.target;
            state.port = record.port;
            state.expiresAt = expiresAt;
            state.ttlMs = ttlToMs(record.ttl);
            appendUniqueAddress(state.sourceAddresses, senderAddress);
            touchedInstances.insert(record.name);
            break;
        }

        case MDNS_RECORDTYPE_TXT: {
            if (!isHttpsServiceInstanceName(record.name)) {
                break;
            }
            auto& state = services_[record.name];
            if (state.serviceType.isEmpty()) {
                state.serviceType = ServiceTypeHttps;
            }
            state.attributes = record.attributes;
            state.expiresAt = expiresAt;
            state.ttlMs = ttlToMs(record.ttl);
            appendUniqueAddress(state.sourceAddresses, senderAddress);
            touchedInstances.insert(record.name);
            break;
        }

        case MDNS_RECORDTYPE_A: {
            auto& host = hosts_[record.name];
            host.expiresAt = expiresAt;
            appendUniqueAddress(host.addresses, record.address);
            break;
        }

        default:
            break;
        }
    }

    if (!touchedInstances.isEmpty()) {
        refreshCandidates(touchedInstances);
    }
}

void MdnsClient::refreshCandidates(const QSet<QByteArray>& touchedInstances)
{
    for (const auto& instanceName : touchedInstances) {
        const auto serviceIt = services_.constFind(instanceName);
        if (serviceIt == services_.cend()) {
            continue;
        }

        const auto& service = serviceIt.value();
        if (service.serviceType != ServiceTypeHttps || service.port <= 0) {
            continue;
        }

        QList<QHostAddress> addresses;
        if (!service.hostName.isEmpty()) {
            const auto hostIt = hosts_.constFind(service.hostName);
            if (hostIt != hosts_.cend()) {
                addresses = hostIt->addresses;
            }
        }

        for (const auto& address : service.sourceAddresses) {
            appendUniqueAddress(addresses, address);
        }

        if (addresses.isEmpty()) {
            continue;
        }

        const bool isHomecloud = service.attributes.value(TxtKeySeagate) == TxtValueHomecloud;
        const auto ttlMs = service.ttlMs > 0 ? static_cast<int>(service.ttlMs) : MinRecordTtlMs;

        for (const auto& address : addresses) {
            if (address.protocol() != QAbstractSocket::IPv4Protocol) {
                continue;
            }

            logAllCandidateEndpoint(address.toString(), service.port, QStringLiteral("mdns"));

            if (isHomecloud) {
                ++diagnostics_.mdnsServiceCandidates;
                if (updateDiscoveredRecord(address.toString(), service.port, ttlMs)) {
                    logAcceptedEndpoint(address.toString(), service.port, QStringLiteral("mdns"));
                    emit resultsChanged_internal(discoveredRecords_.values());
                }
            } else {
                logNotHomecloudEndpoint(address.toString(), service.port, QStringLiteral("mdns"));
            }
        }
    }
}

bool MdnsClient::updateDiscoveredRecord(const QString& address, quint16 port, int ttlMs)
{
    const auto key = QStringLiteral("%1:%2").arg(address, QString::number(port));
    auto& record = discoveredRecords_[key];
    bool changed = false;

    if (record.address != address) {
        record.address = address;
        changed = true;
    }
    if (record.port != port) {
        record.port = port;
        changed = true;
    }
    if (record.origin != DeviceOrigin::MDNS) {
        record.origin = DeviceOrigin::MDNS;
        changed = true;
    }
    if (record.deviceType != DeviceType::Local) {
        record.deviceType = DeviceType::Local;
        changed = true;
    }

    lastSeen_[key] = QDateTime::currentDateTimeUtc();
    recordTtlMs_[key] = std::max(ttlMs, MinRecordTtlMs);
    return changed;
}

QString mdns_record_type_to_str(int t)
{
    switch (t) {
    case MDNS_RECORDTYPE_A:
        return QStringLiteral("A");
    case MDNS_RECORDTYPE_PTR:
        return QStringLiteral("PTR");
    case MDNS_RECORDTYPE_TXT:
        return QStringLiteral("TXT");
    case MDNS_RECORDTYPE_AAAA:
        return QStringLiteral("AAAA");
    case MDNS_RECORDTYPE_SRV:
        return QStringLiteral("SRV");
    case MDNS_RECORDTYPE_NSEC:
        return QStringLiteral("NSEC");
    default:
        return QStringLiteral("UNKNOWN");
    }
}
