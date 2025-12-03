#include "devicetypes.h"
#include "configfile.h"

#include <QMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLoggingCategory>

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
const auto address_C = QStringLiteral("address");
const auto deviceType_C = QStringLiteral("device_type");
const auto deviceOrigin_C = QStringLiteral("device_origin");
const auto port_C = QStringLiteral("port");
const auto device_id_C = QStringLiteral("device_id");
const auto cert_common_name_C = QStringLiteral("certificate_common_name");
const auto friendly_name_C = QStringLiteral("friendly_name");
const auto paths_C = QStringLiteral("paths");
}

Q_LOGGING_CATEGORY(lcDevice, "device", QtInfoMsg)

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

QString DevicePath::devTypeToStr(DeviceType val)
{
    QMap<DeviceType,QString> map = {
        {DeviceType::Unknown, QStringLiteral("unknown")},
        {DeviceType::Local, QStringLiteral("local")},
        {DeviceType::Public, QStringLiteral("public")},
        {DeviceType::Remote, QStringLiteral("remote")}
    };
    if (map.contains(val))
        return map[val];
    return {};
}

DevicePathOrigin DevicePath::strToDevOrigin(const QString& str)
{
    QMap<QString, DevicePathOrigin> map = {
        {QStringLiteral("remote"), DevicePathOrigin::Remote},
        {QStringLiteral("mdns"), DevicePathOrigin::MDNS},
        {QStringLiteral("static"), DevicePathOrigin::Static},
        {QStringLiteral("unknown"), DevicePathOrigin::Unknown},
    };
    if (map.contains(str))
        return map[str];
    return DevicePathOrigin::Unknown;
}

QString DevicePath::originToStr(DevicePathOrigin val)
{
    QMap<DevicePathOrigin,QString> map = {
        {DevicePathOrigin::Unknown, QStringLiteral("unknown")},
        {DevicePathOrigin::MDNS, QStringLiteral("mdns")},
        {DevicePathOrigin::Remote, QStringLiteral("remote")},
        {DevicePathOrigin::Static, QStringLiteral("static")}
    };
    if (map.contains(val))
        return map[val];
    return {};
}

QJsonObject DevicePath::toJson() const
{
    QJsonObject result;
    result[address_C] = address;
    result[deviceType_C] = devTypeToStr(deviceType);
    result[deviceOrigin_C] = originToStr(origin);
    result[port_C] = port;
    return result;
}

DevicePath DevicePath::fromJson(const QJsonObject& val)
{
    DevicePath dp(
        val[address_C].toString(),
        strToDevType(val[deviceType_C].toString()),
        strToDevOrigin(val[deviceOrigin_C].toString()),
        val[port_C].toInt());
    return dp;
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

QString Device::makeServerUrl(const QString &url, int port, bool add_folder, bool add_port)
{
    QString result;

    bool apiOnlyPort = CUR::ConfigFile::useLocalPortForApiOnly();

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
    if (port > 0) {
        if (apiOnlyPort) {
            if (add_port)
                tmpurl.setPort(port);
        }
        else {
            tmpurl.setPort(port);
        }
    }

    //qCInfo(lcDevice) << "Make URL" << url << ":" << port << "--->" << tmpurl.toString();

    return tmpurl.toString();
}

std::optional<DevicePath> Device::findPath(const QUuid &id) const
{
    if (paths.isEmpty()) {
        qCWarning(lcDevice) << "[findPath] No paths defined";
        return std::nullopt;
    }

    for (const auto& p: std::as_const(paths)) {
        if (p.id == id)
            return p;
    }

    qCWarning(lcDevice) << "Path ID" << id << "not found";
    return std::nullopt;
}

std::optional<QUuid> Device::getBestPathId()
{
    return getBestPathId(*this);
}

std::optional<QUuid> Device::getBestPathId(const Device &dev)
{
    if (dev.paths.isEmpty()) {
        qCWarning(lcDevice) << "[getBestPathId] No paths defined";
        return std::nullopt;
    }

    if (dev.paths.size() == 1)
        return dev.paths.first().id;

    QList<DevicePath> paths;
    for (const auto& p: dev.paths) {
        if (p.deviceType != DeviceType::Unknown && p.isOnline) {
            paths.append(p);
        }
    }

    if (paths.isEmpty())
        return dev.paths.first().id;

    if (paths.size() == 1)
        return paths.first().id;

    std::stable_sort(paths.begin(), paths.end(), [&](const DevicePath& a, const DevicePath& b) {
        return a.deviceType < b.deviceType;
    });

    return paths.first().id;
}

Device Device::MakeStatic(const QString &url, const QString &name)
{
    Device dev;
    dev.certificateCommonName = name;
    dev.isStatic = true;
    DevicePath dp(url, DeviceType::Public, DevicePathOrigin::Static, 0);
    dev.paths.append(dp);
    return dev;
}

QJsonDocument Device::toJson(const Device& dev)
{
    QJsonDocument doc;
    QJsonObject obj;
    obj[device_id_C] = dev.seagateDeviceID;
    obj[cert_common_name_C] = dev.certificateCommonName;
    obj[friendly_name_C] = dev.friendlyName;
    obj[hostname_C] = dev.hostname;
    QJsonArray arr;
    for (const auto& dev_path: std::as_const(dev.paths)) {
        arr.append(dev_path.toJson());
    }
    obj[paths_C] = arr;
    doc.setObject(obj);
    return doc;
}

QByteArray Device::toJsonStr(const Device& dev)
{
    const auto doc = Device::toJson(dev);
    return doc.toJson(QJsonDocument::Compact);
}

Device Device::fromJson(const QJsonDocument &doc)
{
    Device d;
    if (doc.isEmpty())
        return {};

    QJsonObject obj = doc.object();
    d.seagateDeviceID = obj[device_id_C].toString();
    d.certificateCommonName = obj[cert_common_name_C].toString();
    d.friendlyName = obj[friendly_name_C].toString();
    d.hostname = obj[hostname_C].toString();
    d.paths = Device::jsonToPaths(obj[paths_C].toArray());

    return d;
}

Device Device::fromJsonStr(const QByteArray &ba)
{
    QJsonDocument doc = QJsonDocument::fromJson(ba);
    return Device::fromJson(doc);
}

QJsonArray Device::pathsToJson(const QList<DevicePath> &devicePaths)
{
    QList<DevicePath> paths;
    QJsonArray array;
    for (const auto& item: devicePaths) {
        array.append(item.toJson());
    }
    return array;
}

QList<DevicePath> Device::jsonToPaths(const QJsonArray& val)
{
    if (val.isEmpty())
        return {};

    QList<DevicePath> ret;
    for (const auto& item: val) {
        auto dev = DevicePath::fromJson(item.toObject());
        ret.append(dev);
    }

    return ret;
}

void Device::setActicvePath(const QUuid &id)
{
    for (auto& p: paths) {
        p.isActive = (p.id == id);
    }
}

void Device::setOnlinePath(const QUuid& id)
{
    for (auto& p: paths) {
        if (p.id == id) {
            p.isOnline = true;
            break;
        }
    }
}

void Device::clearOnlinePaths()
{
    for (auto& p: paths)
        p.isOnline = false;
}
