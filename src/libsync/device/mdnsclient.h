#pragma once

#include "personalcloudlib.h"
#include "device/devicetypes.h"

#include <QObject>
#include <QDateTime>
#include <QHash>
#include <QMap>
#include <QSet>
#include <QTimer>
#include <QUdpSocket>

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

class APPLICATIONSYNC_EXPORT MdnsClient: public QObject
{
    Q_OBJECT

public:
    explicit MdnsClient(QObject* parent = nullptr);
    ~MdnsClient();

    void start();
    void stop();

signals:
    void resultsChanged(const QList<DevicePath>& records);
    void resultsChanged_internal(const QList<DevicePath>& records);

private slots:
    void onReadyRead();

private:
    class ServiceState
    {
    public:
        QByteArray serviceType;
        QByteArray hostName;
        quint16 port = 0;
        quint32 ttlMs = 0;
        QDateTime expiresAt;
        QMap<QByteArray, QByteArray> attributes;
        QList<QHostAddress> sourceAddresses;
    };

    class HostState
    {
    public:
        QDateTime expiresAt;
        QList<QHostAddress> addresses;
    };

    class DiagnosticsState
    {
    public:
        int parseFailures = 0;
        int mdnsPackets = 0;
        int mdnsRecords = 0;
        int mdnsHttpsPtrRecords = 0;
        int mdnsServiceCandidates = 0;
        int notHomecloudMdnsCandidates = 0;
        QSet<QString> allCandidateEndpoints;
        QSet<QString> acceptedEndpoints;
        QSet<QString> notHomecloudCandidateEndpoints;
    };

    void performScanCycle();
    void processMessage(const Message& message, const QHostAddress& senderAddress);
    void refreshCandidates(const QSet<QByteArray>& touchedInstances);
    void resetDiagnostics();
    void logDiscoverySummary(const QString& phase) const;
    void logAllCandidateEndpoint(const QString& address, quint16 port, const QString& source);
    void logAcceptedEndpoint(const QString& address, quint16 port, const QString& source);
    void logNotHomecloudEndpoint(const QString& address, quint16 port, const QString& source);
    void setupSockets();
    void joinMulticastGroups();
    bool pruneExpiredState();
    bool updateDiscoveredRecord(const QString& address, quint16 port, int ttlMs);

    QUdpSocket* multicastSocket_ = nullptr;
    QUdpSocket* unicastSocket_ = nullptr;
    QSet<QString> joinedMulticastIfaces_;
    QHash<QByteArray, ServiceState> services_;
    QHash<QByteArray, HostState> hosts_;
    QMap<QString, DevicePath> discoveredRecords_;
    QMap<QString, QDateTime> lastSeen_;
    QMap<QString, int> recordTtlMs_;
    QTimer scanTimer_;
    QTimer debounceTimer_;
    QTimer notFoundTimer_;
    QList<DevicePath> lastData_;
    DiagnosticsState diagnostics_;
};

QString mdns_record_type_to_str(int t);
