#include "devicetypes.h"

#include <QMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace {
const QString certificate_common_name_C() {return QStringLiteral("certificate_common_name");}
const QString date_C() {return QStringLiteral("date");}
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

const QString oobe_C() {return QStringLiteral("OOBE");}
const QString oobe_done_C() {return QStringLiteral("done");}
const QString apps_C() {return QStringLiteral("apps");}
const QString apps_files_C() {return QStringLiteral("files");}
const QString apps_photos_C() {return QStringLiteral("photos");}
const QString state_C() {return QStringLiteral("state");}
}

DeviceType DevicePath::strToDevType(const QString &str)
{
    QMap<QString, DeviceType> map = {
        {QStringLiteral("local"), DeviceType::Local},
        {QStringLiteral("public"), DeviceType::Public},
        {QStringLiteral("remote"), DeviceType::Remote}
    };

    if (map.contains(str))
        return map[str];
    return DeviceType::Unknown;
}

DeviceInfoAbout DeviceInfoAbout::fromJson(const QJsonDocument &doc)
{
    DeviceInfoAbout di;
    di.certificate_common_name = doc[certificate_common_name_C()].toString();
    di.date = QDateTime::fromString(doc[date_C()].toString(), Qt::ISODateWithMs);
    di.default_mac_addr = doc[default_mac_addr_C()].toString();
    di.hostname = doc[hostname_C()].toString();
    di.install_id = doc[install_id_C()].toString();
    di.model_name = doc[model_name_C()].toString();
    di.model_number = doc[model_number_C()].toString();
    di.os_state = doc[os_state_C()].toString();
    di.product_id = doc[product_id_C()].toString();
    di.serial_number = doc[serial_number_C()].toString();
    di.version = doc[version_C()].toString();

    const auto& obj = doc[hardware_info_C()].toObject();
    di.hardware_info.memory = obj[memory_C()].toInteger(0);
    di.hardware_info.processor_count = obj[processor_count_C()].toInteger(0);
    di.hardware_info.processor_type = obj[processor_type_C()].toString();

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

        di.network_interfaces.append(iface);
    }
    return di;
}

DeviceInfoStatus DeviceInfoStatus::fromJson(const QJsonDocument &doc)
{
    DeviceInfoStatus ds;
    const auto& oobe = doc[oobe_C()].toObject();
    ds.oobe_done = oobe[oobe_done_C()].toBool();

    const auto& apps = doc[apps_C()].toObject();
    ds.app_files = apps[apps_files_C()].toString();
    ds.app_photos = apps[apps_photos_C()].toString();

    ds.state = doc[state_C()].toString();
    return ds;
}

QString normalizeUrl(const QString &url, int port, bool add_folder)
{
    QString result;

    if (!url.startsWith(QStringLiteral("http://")) && !url.startsWith(QStringLiteral("https://"))) {
        result = QStringLiteral("https://");
    }

    if (add_folder) {
        if (url.endsWith(QStringLiteral("files")) || url.endsWith(QStringLiteral("files/"))) {
            result += url;
            if (!result.endsWith(QStringLiteral("/")))
                result += QStringLiteral("/");
        }
        else {
            result += url;
            if (result.endsWith(QStringLiteral("/")))
                result += QStringLiteral("files/");
            else
                result += QStringLiteral("/files/");
        }
    }
    else {
        result += url;
        if (!result.endsWith(QStringLiteral("/")))
            result += QStringLiteral("/");
    }

    QUrl tmpurl(result);
    if (port > 0)
        tmpurl.setPort(port);

    return tmpurl.toString();
}

std::optional<DevicePath> Device::firstRemotePath(const Device &dev)
{
    if (dev.paths.isEmpty())
        return std::nullopt;
    for (const auto& p: dev.paths) {
        if (p.deviceType != DeviceType::Local)
            return p;
    }
    return std::nullopt;
}
