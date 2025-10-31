#include "mdnsclient.h"

#include <QNetworkInterface>
#include <QTimer>
#include <QRestReply>
#include <QLoggingCategory>
#include <QJsonObject>
#include <QJsonArray>

namespace {
const QString api_dev_about = QStringLiteral("/api/v1/about"); // GET

const QString certificate_common_name_C() {return QStringLiteral("certificate_common_name");}
//const QString date_C() {return QStringLiteral("date");}
const QString default_mac_addr_C() {return QStringLiteral("default_mac_addr");}
const QString hostname_C() {return QStringLiteral("hostname");}
const QString install_id_C() {return QStringLiteral("install_id");}
const QString model_name_C() {return QStringLiteral("model_name");}
const QString model_number_C() {return QStringLiteral("model_number");}
const QString os_state_C() {return QStringLiteral("os_state");}
const QString product_id_C() {return QStringLiteral("product_id");}
const QString serial_number_C() {return QStringLiteral("serial_number");}
const QString version_C() {return QStringLiteral("version");}
const QString hardware_info_C() {return QStringLiteral("hardware_info");}
const QString memory_C() {return QStringLiteral("memory");}
const QString processor_count_C() {return QStringLiteral("processor_count");}
const QString processor_type_C() {return QStringLiteral("processor_type");}
const QString network_interfaces_C() {return QStringLiteral("network_interfaces");}
const QString link_C() {return QStringLiteral("link");}
const QString mac_address_C() {return QStringLiteral("mac_address");}
const QString name_C() {return QStringLiteral("name");}
const QString type_C() {return QStringLiteral("type");}
const QString ipv4_info_C() {return QStringLiteral("ipv4_info");}
const QString ipv4_C() {return QStringLiteral("ipv4");}
const QString gateway_C() {return QStringLiteral("gateway");}
const QString netmask_C() {return QStringLiteral("netmask");}
}

Q_LOGGING_CATEGORY(lcMdnsDevice, "mdns.device", QtDebugMsg)

namespace CUR {

MdnsClient::MdnsClient(QObject* parent)
    : QObject(parent)
    , rest_mgr(std::make_unique<QRestAccessManager>(&net_mgr))
{
    dns = new QDnsLookup(this);
    rest_factory = std::make_unique<QNetworkRequestFactory>();

    connect(&net_mgr, &QNetworkAccessManager::sslErrors, this, [&](QNetworkReply *reply, const QList<QSslError> &errors) {
        reply->ignoreSslErrors();
    });

    connect(this, &MdnsClient::device_info_finished, this, [&](std::optional<QJsonDocument> doc, int code) {
        if (!doc) {
            qCWarning(lcMdnsDevice) << "device info returns empty data, code" << code;
            return;
        }
        if (code == 200) {
            LocalDeviceInfo localDevice;
            parse_device_about(doc.value(), localDevice);
            for (int i = 0; i < records_.size(); i++) {
                if (records_[i] == processingDevice) {
                    records_[i].name = localDevice.certificate_common_name;
                }
            }

            query_device_info();
            return;
        }

        emit requestCompleted();
    });

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
        }

        deviceRequestQueue = records_;

        query_device_info();
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

void MdnsClient::query_device_info()
{
    if (deviceRequestQueue.isEmpty()) {
        processingDevice = {};
        emit requestCompleted();
        return;
    }

    processingDevice = deviceRequestQueue.takeFirst();
    QUrl url;
    url = QUrl::fromUserInput(processingDevice.host);
    url.setScheme(QStringLiteral("https"));
    if (processingDevice.port > 0)
        url.setPort(processingDevice.port);

    rest_factory->setBaseUrl(url);

    auto req = rest_factory->createRequest(api_dev_about);

    rest_mgr->get(req, this, [&](QRestReply &reply) {
        emit device_info_finished(reply.readJson(), reply.httpStatus());
    });
}

void MdnsClient::parse_device_about(const QJsonDocument &doc, LocalDeviceInfo& info)
{
    info = {};
    info.certificate_common_name = doc[certificate_common_name_C()].toString();

    // "2025-10-02T12:20:08.872411615Z"
    //info.date = QDateTime::fromString(doc["date"].toString(), "yyyy-MM-ddThh:mm:ss.zzzzzzzzzZ");

    info.default_mac_addr = doc[default_mac_addr_C()].toString();
    info.hostname = doc[hostname_C()].toString();
    info.install_id = doc[install_id_C()].toString();
    info.model_name = doc[model_name_C()].toString();
    info.model_number = doc[model_number_C()].toString();
    info.os_state = doc[os_state_C()].toString();
    info.product_id = doc[product_id_C()].toString();
    info.serial_number = doc[serial_number_C()].toString();
    info.version = doc[version_C()].toString();

    const auto& obj = doc[hardware_info_C()].toObject();
    info.hardware_info.memory = obj[memory_C()].toInteger(0);
    info.hardware_info.processor_count = obj[processor_count_C()].toInteger(0);
    info.hardware_info.processor_type = obj[processor_type_C()].toString();

    const auto& interfaces = doc[network_interfaces_C()].toArray();
    for (auto item: interfaces) {
        LocalDeviceInterface iface;
        auto element = item.toObject();
        iface.link = element[link_C()].toString();
        iface.mac_address = element[mac_address_C()].toString();
        iface.name = element[name_C()].toString();
        iface.type = element[type_C()].toString();

        const auto& ipinfo = element[ipv4_info_C()].toObject();

        iface.ipv4_info.ipv4 = ipinfo[ipv4_C()].toString();
        iface.ipv4_info.gateway = ipinfo[gateway_C()].toString();
        iface.ipv4_info.netmask = ipinfo[netmask_C()].toString();

        info.network_interfaces.append(iface);
    }
}

} // namespace CUR
