#include "mdnsclient.h"

#include <QNetworkInterface>
#include <QTimer>
#include <QRestReply>
#include <QLoggingCategory>
#include <QJsonObject>
#include <QJsonArray>

Q_LOGGING_CATEGORY(lcMdnsDevice, "mdns.device", QtDebugMsg)

namespace CUR {

MdnsClient::MdnsClient(QObject* parent)
    : QObject(parent)
{
    dns = new QDnsLookup(this);

    connect(dns, &QDnsLookup::finished, this, [&] {
        qCDebug(lcMdnsDevice) << "mDNS lookup finished";

        if (dns->error() == QDnsLookup::NoError) {
            const auto srv_records = dns->serviceRecords();
            for (const QDnsServiceRecord& record : srv_records) {
                MdnsRecord rec;
                rec.name = fixHost(record.name());
                rec.host = record.target();
                rec.port = record.port();
                records_.append(rec);
            }
        }
        requestCompleted();
    });
}

MdnsClient::~MdnsClient()
{
}

void MdnsClient::query()
{
    records_.clear();

    dns->setType(QDnsLookup::PTR);
    dns->setName(QStringLiteral("_https._tcp"));
    dns->lookup();
}

QString MdnsClient::fixHost(const QString &host)
{
    QString h(host);
    if (h.endsWith(QStringLiteral("."))) {
        h = h.left(h.length() - 1);
    }
    return h;
}

QUrl MdnsClient::constructUrlFromLocalDomain(const QString &url, int port)
{
    QUrl result;
    result = QUrl::fromUserInput(url);
    result.setScheme(QStringLiteral("https"));
    if (port > 0)
        result.setPort(port);
    return result;
}

} // namespace CUR
