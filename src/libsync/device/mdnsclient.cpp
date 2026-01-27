#include "mdnsclient.h"

#include <QNetworkInterface>
#include <QTimer>
#include <QRestReply>
#include <QLoggingCategory>
#include <QJsonObject>
#include <QJsonArray>

Q_LOGGING_CATEGORY(lcMdnsDevice, "mdns.device", QtDebugMsg)

namespace {

const unsigned char request_data[] = {
    // Query ID
    0x00, 0x00,
    // Flags
    0x00, 0x00,
    // 1 question
    0x00, 0x01,
    // No answer, authority or additional RRs
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // _https._tcp.local.
    0x06, '_', 'h', 't', 't', 'p', 's',
    0x04, '_', 't', 'c', 'p', 0x05, 'l', 'o', 'c', 'a', 'l', 0x00,
    // PTR record
    0x00, MDNS_RECORDTYPE_PTR,
    // QU (unicast response) and class IN
    0x80, MDNS_CLASS_IN};

template<class T>
bool parseInt(const QByteArray& packet, quint16& offset, T& value)
{
    if (offset + sizeof(T) > static_cast<unsigned int>(packet.length()))
        return false;

    value = qFromBigEndian<T>(reinterpret_cast<const uchar*>(packet.constData() + offset));
    offset += sizeof(T);
    return true;
}

bool parseName(const QByteArray& packet, quint16& offset, QByteArray& name)
{
    quint16 offset_end = 0;
    quint16 offset_ptr = offset;

    forever {
        quint8 bytes_count = 0;
        if (!parseInt<quint8>(packet, offset, bytes_count))
            return false;

        if (!bytes_count)
            break;

        switch (bytes_count & 0xc0) {
        case 0x00:
            if (offset + bytes_count > packet.length())
                return false;  // too long

            name.append(packet.mid(offset, bytes_count));
            name.append('.');
            offset += bytes_count;
            break;

        case 0xc0:
        {
            quint8 bytes_count2 = 0;
            quint16 new_offset = 0;
            if (!parseInt<quint8>(packet, offset, bytes_count2))
                return false;

            new_offset = ((bytes_count & ~0xc0) << 8) | bytes_count2;
            if (new_offset >= offset_ptr)
                return false;  // prevent infinite loop

            offset_ptr = new_offset;
            if (!offset_end)
                offset_end = offset;

            offset = new_offset;
            break;
        }

        default:
            return false;  // no other types supported
        }
    }

    if (offset_end)
        offset = offset_end;

    return true;
}

bool parseRecord(QByteArray const& packet, quint16& offset, DnsRecord& record)
{
    QByteArray name;
    quint16 type = 0;
    quint16 class_ = 0;
    quint16 dataLen = 0;
    quint32 ttl = 0;

    if (! parseName(packet, offset, name) ||
        ! parseInt<quint16>(packet, offset, type) ||
        ! parseInt<quint16>(packet, offset, class_) ||
        ! parseInt<quint32>(packet, offset, ttl) ||
        ! parseInt<quint16>(packet, offset, dataLen) )
    {
        return false;
    }

    record.name = name;
    record.type = type;
    record.flushCache = class_ & 0x8000;
    record.ttl = ttl;

    switch (type) {
    case MDNS_RECORDTYPE_A:
    {
        quint32 ipv4Addr = 0;
        if (!parseInt<quint32>(packet, offset, ipv4Addr))
            return false;

        record.address = QHostAddress(ipv4Addr);
        break;
    }
    case MDNS_RECORDTYPE_AAAA:
    {
        if (offset + 16 > packet.length())
            return false;
        // skip IPv6
        offset += 16;
        break;
    }
    case MDNS_RECORDTYPE_NSEC:
    {
        QByteArray nextDomainName;
        quint8 number;
        quint8 length;
        if (!parseName(packet, offset, nextDomainName)
            || !parseInt<quint8>(packet, offset, number)
            || !parseInt<quint8>(packet, offset, length)
            || (number != 0)
            || (offset + length > packet.length()) )
        {
            return false;
        }

        // Skip
        offset += length;
        break;
    }
    case MDNS_RECORDTYPE_PTR:
    {
        QByteArray target;
        if (!parseName(packet, offset, target))
            return false;

        record.target = target;
        break;
    }
    case MDNS_RECORDTYPE_SRV:
    {
        quint16 priority = 0;
        quint16 weight = 0;
        quint16 port = 0;
        QByteArray target;
        if (!parseInt<quint16>(packet, offset, priority)
            || !parseInt<quint16>(packet, offset, weight)
            || !parseInt<quint16>(packet, offset, port)
            || !parseName(packet, offset, target) )
        {
            return false;
        }

        record.priority = priority;
        record.weight = weight;
        record.port = port;
        record.target = target;
        break;
    }
    case MDNS_RECORDTYPE_TXT:
    {
        quint16 start = offset;
        while (offset < start + dataLen) {
            quint8 nBytes = 0;
            if (!parseInt<quint8>(packet, offset, nBytes) || (offset + nBytes > packet.length()))
            {
                return false;
            }

            if (nBytes == 0)
                break;

            QByteArray const attr(packet.constData() + offset, nBytes);
            offset += nBytes;
            qsizetype const splitIndex = attr.indexOf('=');
            if (splitIndex == -1)
                record.attributes.insert(attr, QByteArray());
            else
                record.attributes.insert(attr.left(splitIndex), attr.mid(splitIndex + 1));
        }
        break;
    }

    default:
        offset += dataLen;
        break;
    }
    return true;
}

bool fromPacket(QByteArray const& packet, Message& message)
{
    quint16 offset = 0;
    quint16 transactionId, flags, nQuestion, nAnswer, nAuthority, nAdditional;
    if (!parseInt<quint16>(packet, offset, transactionId) ||
        !parseInt<quint16>(packet, offset, flags) ||
        !parseInt<quint16>(packet, offset, nQuestion) ||
        !parseInt<quint16>(packet, offset, nAnswer) ||
        !parseInt<quint16>(packet, offset, nAuthority) ||
        !parseInt<quint16>(packet, offset, nAdditional))
    {
        return false;
    }

    message.transactionId = transactionId;
    message.isResponse = flags & 0x8400;
    message.isTruncated = flags & 0x0200;

    quint16 const nRecord = nAnswer + nAuthority + nAdditional;
    for (quint16 i = 0; i < nRecord; ++i) {
        DnsRecord record;
        if (!parseRecord(packet, offset, record)) {
            return false;
        }

        if (record.port > 0)
            message.port = record.port;

        if (record.attributes.contains(QStringLiteral("seagate").toUtf8())) {
            if (record.attributes[QStringLiteral("seagate").toUtf8()] == QStringLiteral("homecloud").toUtf8()) {
                message.isHomecloud = true;
            }
        }

    }
    return true;
}

} // anonymous namespace


MdnsClient::MdnsClient(QObject *parent)
    : QObject(parent)
{
    setupSockets();
    scanTimer_.setInterval(5000);
    connect(&scanTimer_, &QTimer::timeout, this, &MdnsClient::performScanCycle);
    debounceTimer_.setInterval(2000);
    debounceTimer_.setSingleShot(true);
    notFoundTimer_.setInterval(3000);
    notFoundTimer_.setSingleShot(true);

    connect(&notFoundTimer_, &QTimer::timeout, this, [this] {
        debounceTimer_.start();
    });

    connect(&debounceTimer_, &QTimer::timeout, this, [this] {
        notFoundTimer_.stop();
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
    for (auto socket: sockets) {
        socket->abort();
    }
}

void MdnsClient::start()
{
    if (scanTimer_.isActive()) {
        scanTimer_.stop();
    }

    discoveredRecords_.clear();
    lastSeen_.clear();
    notFoundTimer_.start();

    performScanCycle();
    scanTimer_.start();
}

void MdnsClient::stop()
{
    scanTimer_.stop();
}

void MdnsClient::onReadyRead()
{
    auto* sender_socket = qobject_cast<QUdpSocket*>(sender());
    if (!sender_socket)
        return;

    bool changed = false;
    while (sender_socket->hasPendingDatagrams()) {
        QByteArray packet;
        packet.resize(sender_socket->pendingDatagramSize());

        QHostAddress senderAddress;
        quint16 senderPort = 0;

        sender_socket->readDatagram(packet.data(), packet.size(), &senderAddress, &senderPort);

        Message message;
        if (fromPacket(packet, message) && message.isHomecloud) {

            QString key = QStringLiteral("%1:%2").arg(senderAddress.toString()).arg(message.port);
            lastSeen_[key] = QDateTime::currentDateTime();

            if (!discoveredRecords_.contains(key)) {
                DevicePath rec;
                rec.origin = DeviceOrigin::MDNS;
                rec.deviceType = DeviceType::Local;
                rec.address = senderAddress.toString();
                rec.port = message.port;
                discoveredRecords_.insert(key, rec);
                changed = true;
            }

            if (changed) {
                emit resultsChanged_internal(discoveredRecords_.values());
            }
        }
    }
}

void MdnsClient::createSocket(const QHostAddress &addr)
{
    auto socket = new QUdpSocket(this);
    if (socket->bind(addr, 0, QAbstractSocket::ShareAddress|QAbstractSocket::ReuseAddressHint)) {
        connect(socket, &QUdpSocket::readyRead, this, &MdnsClient::onReadyRead);
        sockets.append(socket);
    }
    else {
        socket->deleteLater();
    }
}

void MdnsClient::performScanCycle()
{
    bool changed = false;
    auto now = QDateTime::currentDateTime();

    // Check timeout and delete no answer more than 12 seconds
    // 12 seconds ~> 2 query cycle (5+5)
    auto it = lastSeen_.begin();
    while (it != lastSeen_.end()) {
        if (it.value().secsTo(now) > 12) {
            discoveredRecords_.remove(it.key());
            it = lastSeen_.erase(it);
            changed = true;
        } else {
            ++it;
        }
    }

    if (changed) {
        emit resultsChanged_internal(discoveredRecords_.values());
    }

    // new request
    QByteArray ba(reinterpret_cast<const char*>(request_data), sizeof(request_data));
    for (auto socket : std::as_const(sockets)) {
        socket->writeDatagram(ba, QHostAddress(QStringLiteral("224.0.0.251")), 5353);
    }
}

void MdnsClient::setupSockets()
{
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const auto& iface : interfaces) {
        if (!iface.isValid() || !iface.flags().testFlag(QNetworkInterface::IsRunning))
            continue;

        for (const auto& entry : iface.addressEntries()) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                auto socket = new QUdpSocket(this);
                if (socket->bind(entry.ip(), 0, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
                    connect(socket, &QUdpSocket::readyRead, this, &MdnsClient::onReadyRead);
                    qDebug(lcMdnsDevice) << "Socket binded to" << entry.ip();
                    sockets.append(socket);
                } else {
                    qDebug(lcMdnsDevice) << "Failed bind socket to" << entry.ip();
                    socket->deleteLater();
                }
            }
        }
    }
}

