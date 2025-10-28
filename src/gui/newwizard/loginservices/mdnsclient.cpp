#include "mdnsclient.h"

#include <QNetworkInterface>
#include <QTimer>

namespace CUR {

MdnsClient::MdnsClient(QObject* parent)
    : QObject(parent)
{
    dns = new QDnsLookup(this);
    connect(dns, &QDnsLookup::finished, this, [&] {
        qDebug() << "DNS lookup finished";

        if (dns->error() == QDnsLookup::NoError) {

            qDebug() << "serviceRecords";
            const auto srv_records = dns->serviceRecords();
            for (const QDnsServiceRecord& record : srv_records) {
                //qDebug() << record.name() << record.port() << record.target();
                DeviceInfo di;
                di.name = record.name();
                di.host = record.target();
                di.port = record.port();
                records_.append(di);
            }

            qDebug() << "hostAddressRecords";
            const auto addr_records = dns->hostAddressRecords();
            for (const QDnsHostAddressRecord& record : addr_records) {
                qDebug() << record.name() << record.value();
            }

            qDebug() << "nameServerRecords";
            const auto ns_records = dns->nameServerRecords();
            for (const QDnsDomainNameRecord& record : ns_records) {
                qDebug() << record.name() << record.value();
            }

            qDebug() << "pointerRecords";
            const auto pt_records = dns->pointerRecords();
            for (const QDnsDomainNameRecord& record : pt_records) {
                qDebug() << record.name() << record.value();
            }

            qDebug() << "textRecords";
            const auto txt_records = dns->textRecords();
            for (const QDnsTextRecord& record : txt_records) {
                qDebug() << record.name() << record.values();
            }
        }

        emit requestCompleted();
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

} // namespace CUR
