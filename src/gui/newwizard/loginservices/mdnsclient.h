#pragma once

#include "device/devicetypes.h"

#include <QObject>
#include <QUdpSocket>
#include <QTimer>

enum mdns_record_type {
    MDNS_RECORDTYPE_IGNORE = 0,
    // Address
    MDNS_RECORDTYPE_A = 1,
    // Domain Name pointer
    MDNS_RECORDTYPE_PTR = 12,
    // Arbitrary text string
    MDNS_RECORDTYPE_TXT = 16,
    // IP6 Address [Thomson]
    MDNS_RECORDTYPE_AAAA = 28,
    // Server Selection [RFC2782]
    MDNS_RECORDTYPE_SRV = 33,
    // List of records
    MDNS_RECORDTYPE_NSEC = 47
};

enum mdns_entry_type {
    MDNS_ENTRYTYPE_QUESTION = 0,
    MDNS_ENTRYTYPE_ANSWER = 1,
    MDNS_ENTRYTYPE_AUTHORITY = 2,
    MDNS_ENTRYTYPE_ADDITIONAL = 3
};

enum mdns_class { MDNS_CLASS_IN = 1 };

class DnsRecord
{
public:
    QByteArray name;
    quint16 type {0};
    bool flushCache {false};
    quint32 ttl {3600};

    QHostAddress address;
    QByteArray target;
    QByteArray nextDomainName;
    quint16 priority {0};
    quint16 weight {0};
    quint16 port {0};
    QMap<QByteArray, QByteArray> attributes;
};

class Message
{
public:
    QHostAddress address;
    quint16 port {0};
    quint16 transactionId {0};
    bool isResponse {false};
    bool isTruncated {false};
    bool isHomecloud {false};
    QList<DnsRecord> records;
};

namespace CUR {

class MdnsClient: public QObject
{
    Q_OBJECT

public:
    explicit MdnsClient(QObject* parent = nullptr);
    ~MdnsClient();

    void query();

    [[nodiscard]] const QList<MdnsRecord>& records() {return records_;}

signals:
    void requestCompleted();
    void message(const QString& msg);
    void messageReceived(Message msg);

private slots:
    void onReadyRead();
    void onMessageReveived(Message msg);

private:
    void createSocket(const QHostAddress& addr);

    QList<MdnsRecord> records_;
    QList<QUdpSocket*> sockets;
    QTimer timer_;
};

} // namespace CUR

QString mdns_record_type_to_str(int t);
