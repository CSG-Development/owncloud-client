#include "devicetypes.h"

#include <QMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace {
const auto certificate_common_name_C = QStringLiteral("certificate_common_name");
const auto date_C = QStringLiteral("date");
const auto default_mac_addr_C = QStringLiteral("default_mac_addr");
const auto hostname_C = QStringLiteral("hostname");
const auto install_id_C = QStringLiteral("install_id");
const auto model_name_C = QStringLiteral("model_name");
const auto model_number_C = QStringLiteral("model_number");
const auto os_state_C = QStringLiteral("os_state");
const auto product_id_C = QStringLiteral("product_id");
const auto serial_number_C = QStringLiteral("serial_number");
const auto version_C = QStringLiteral("version");
const auto hardware_info_C = QStringLiteral("hardware_info");
const auto memory_C = QStringLiteral("memory");
const auto processor_count_C = QStringLiteral("processor_count");
const auto processor_type_C = QStringLiteral("processor_type");
const auto network_interfaces_C = QStringLiteral("network_interfaces");
const auto link_C = QStringLiteral("link");
const auto mac_address_C = QStringLiteral("mac_address");
const auto name_C = QStringLiteral("name");
const auto type_C = QStringLiteral("type");
const auto ipv4_info_C = QStringLiteral("ipv4_info");
const auto ipv4_C = QStringLiteral("ipv4");
const auto gateway_C = QStringLiteral("gateway");
const auto netmask_C = QStringLiteral("netmask");

const auto oobe_C = QStringLiteral("OOBE");
const auto oobe_done_C = QStringLiteral("done");
const auto apps_C = QStringLiteral("apps");
const auto apps_files_C = QStringLiteral("files");
const auto apps_photos_C = QStringLiteral("photos");
const auto state_C = QStringLiteral("state");
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
    di.certificate_common_name = doc[certificate_common_name_C].toString();
    di.date = QDateTime::fromString(doc[date_C].toString(), Qt::ISODateWithMs);
    di.default_mac_addr = doc[default_mac_addr_C].toString();
    di.hostname = doc[hostname_C].toString();
    di.install_id = doc[install_id_C].toString();
    di.model_name = doc[model_name_C].toString();
    di.model_number = doc[model_number_C].toString();
    di.os_state = doc[os_state_C].toString();
    di.product_id = doc[product_id_C].toString();
    di.serial_number = doc[serial_number_C].toString();
    di.version = doc[version_C].toString();

    const auto& obj = doc[hardware_info_C].toObject();
    di.hardware_info.memory = obj[memory_C].toInteger(0);
    di.hardware_info.processor_count = obj[processor_count_C].toInteger(0);
    di.hardware_info.processor_type = obj[processor_type_C].toString();

    const auto& interfaces = doc[network_interfaces_C].toArray();
    for (auto item: interfaces) {
        LocalDeviceInterface iface;
        auto element = item.toObject();
        iface.link = element[link_C].toString();
        iface.mac_address = element[mac_address_C].toString();
        iface.name = element[name_C].toString();
        iface.type = element[type_C].toString();

        const auto& ipinfo = element[ipv4_info_C].toObject();

        iface.ipv4_info.ipv4 = ipinfo[ipv4_C].toString();
        iface.ipv4_info.gateway = ipinfo[gateway_C].toString();
        iface.ipv4_info.netmask = ipinfo[netmask_C].toString();

        di.network_interfaces.append(iface);
    }
    return di;
}

DeviceInfoStatus DeviceInfoStatus::fromJson(const QJsonDocument &doc)
{
    DeviceInfoStatus ds;
    const auto& oobe = doc[oobe_C].toObject();
    ds.oobe_done = oobe[oobe_done_C].toBool();

    const auto& apps = doc[apps_C].toObject();
    ds.app_files = apps[apps_files_C].toString();
    ds.app_photos = apps[apps_photos_C].toString();

    ds.state = doc[state_C].toString();
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
