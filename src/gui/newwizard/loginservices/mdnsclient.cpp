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

namespace CUR {


MdnsClient::MdnsClient(QObject *parent)
    : QObject(parent)
{
    const auto all = QNetworkInterface::allInterfaces();
    for (const auto& item: all) {
        if (!item.isValid() || !item.flags().testFlag(QNetworkInterface::IsRunning))
            continue;

        for (const auto& ip: item.addressEntries()) {
            if (ip.ip().protocol() == QAbstractSocket::IPv6Protocol)
                continue;

            createSocket(ip.ip());
        }
    }

    timer_.setInterval(2 * 1000);
    timer_.setSingleShot(true);
    connect(&timer_, &QTimer::timeout, this, [&] {
        emit requestCompleted();
    });

    connect(this, &MdnsClient::messageReceived, this, &MdnsClient::onMessageReveived);
}

MdnsClient::~MdnsClient()
{
    for (auto socket: std::as_const(sockets)) {
        socket->abort();
    }
}

void MdnsClient::query()
{
    qCDebug(lcMdnsDevice) << "Starting mDNS query";
    records_.clear();
    QByteArray ba(reinterpret_cast<const char*>(request_data), sizeof(request_data));

    timer_.start();
    for (auto socket: std::as_const(sockets)) {
        socket->writeDatagram(ba, QHostAddress(QStringLiteral("224.0.0.251")), 5353);
    }
}

void MdnsClient::onReadyRead()
{
    QUdpSocket* sender_socket = qobject_cast<QUdpSocket*>(sender());
    QByteArray packet;
    packet.resize(sender_socket->pendingDatagramSize());

    QHostAddress address;
    quint16 port = 0;
    sender_socket->readDatagram(packet.data(), packet.size(), &address, &port);

    Message message;
    if (fromPacket(packet, message)) {
        message.address = address;
        emit messageReceived(message);
    }
}

void MdnsClient::onMessageReveived(Message msg)
{
    MdnsRecord rec;
    if (msg.isHomecloud) {
        rec.host = msg.address.toString();
        rec.port = msg.port;
    }
    records_.append(rec);
}

void MdnsClient::createSocket(const QHostAddress &addr)
{
    auto socket = new QUdpSocket(this);
    if (socket->bind(addr, 0, QAbstractSocket::ShareAddress|QAbstractSocket::ReuseAddressHint)) {
        connect(socket, &QUdpSocket::readyRead, this, &MdnsClient::onReadyRead);
        sockets.append(socket);
    }
}

} // namespace CUR
