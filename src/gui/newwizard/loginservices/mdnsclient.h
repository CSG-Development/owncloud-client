#pragma once

#include "device/devicetypes.h"

#include <QObject>
#include <QDnsLookup>

namespace CUR {

class MdnsClient: public QObject
{
    Q_OBJECT

public:
    explicit MdnsClient(QObject* parent = nullptr);
    ~MdnsClient();

    void query();

    [[nodiscard]] const QList<MdnsRecord>& records() {return records_;}
    // Remove end '.'
    static QString fixHost(const QString& host);

signals:
    void requestCompleted();

private:
    static QUrl constructUrlFromLocalDomain(const QString& url, int port);

private:
    QDnsLookup* dns = nullptr;
    QList<MdnsRecord> records_;
    QMap<QString,QStringList> ptr_url;
};

} // namespace CUR
