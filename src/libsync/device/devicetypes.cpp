#include "devicetypes.h"

#include <QMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLoggingCategory>

namespace {
const auto certificate_common_name_C = QStringLiteral("certificate_common_name");
const auto name_C = QStringLiteral("name");
const auto type_C = QStringLiteral("type");
const auto hostname_C = QStringLiteral("hostname");

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

DevicePath::DevicePath()
{
    id = QUuid::createUuid();
}

DevicePath::DevicePath(const QString &addr, DeviceType devType, DeviceOrigin org, int pport)
    : address(addr)
    , deviceType(devType)
    , origin(org)
    , port(pport)
{
    id = QUuid::createUuid();
}

QJsonObject DevicePath::toJson() const
{
    QJsonObject result;
    result[address_C] = address;
    result[deviceType_C] = DevHelpers::devTypeToStr(deviceType);
    result[deviceOrigin_C] = DevHelpers::originToStr(origin);
    result[port_C] = port;
    return result;
}

DevicePath DevicePath::fromJson(const QJsonObject& val)
{
    DevicePath dp(
        val[address_C].toString(),
        DevHelpers::strToDevType(val[deviceType_C].toString()),
        DevHelpers::strToDevOrigin(val[deviceOrigin_C].toString()),
        val[port_C].toInt());
    return dp;
}

int DevicePath::pathPriority(DeviceType devType)
{
    switch (devType) {
    case DeviceType::Local: return 1000;
    case DeviceType::Public: return 100;
    case DeviceType::Remote: return 10;
    case DeviceType::Unknown: return 0;
    }
    return 0;
}

QString DevicePath::toString() const
{
    QStringList l;
    l << QStringLiteral("DevicePath{");
    l << QStringLiteral("address:%1,").arg(address);
    l << QStringLiteral("port:%1,").arg(port);
    l << QStringLiteral("deviceType:%1,").arg(DevHelpers::devTypeToStr(deviceType));
    l << QStringLiteral("origin:%1,").arg(DevHelpers::originToStr(origin));
    l << QStringLiteral("online:%1,").arg(isOnline);
    l << QStringLiteral("active:%1,").arg(isActive);
    l << QStringLiteral("id:%1,").arg(id.toString());
    l << QStringLiteral("%1,").arg(about.toString());
    l << status.toString();
    l << QStringLiteral("}");
    return l.join(QStringLiteral(""));
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

DevicePath *Device::getPathPtr(const QUuid &id)
{
    if (paths.isEmpty()) {
        qCWarning(lcDevice) << "[getPathPtr] No paths defined";
        return nullptr;
    }

    for (int i = 0; i < paths.size(); i++) {
        if (paths[i].id == id)
            return &(paths[i]);
    }

    qCWarning(lcDevice) << "Path ID" << id << "not found";
    return nullptr;
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

    // Only one path exist
    if (dev.paths.size() == 1)
        return dev.paths.first().id;

    QList<DevicePath> paths;
    for (const auto& p: dev.paths) {
        if (p.deviceType != DeviceType::Unknown && p.status.oobe_done) {
            paths.append(p);
        }
    }

    // No online paths found
    if (paths.isEmpty())
        return dev.paths.first().id;

    if (paths.size() == 1)
        return paths.first().id;

    std::stable_sort(paths.begin(), paths.end(), [&](const DevicePath& a, const DevicePath& b) {
        return DevicePath::pathPriority(a.deviceType) > DevicePath::pathPriority(b.deviceType);
    });

    return paths.first().id;
}

std::optional<QUuid> Device::getRemoteOnlyPath() const
{
    if (paths.isEmpty()) {
        qCWarning(lcDevice) << "[getRemoteOnlyPath] No paths defined";
        return std::nullopt;
    }

    QList<DevicePath> plist;
    for (const auto &p: std::as_const(paths)) {
        if (p.deviceType == DeviceType::Remote) {
            plist.append(p);
        }
    }
    if (plist.isEmpty()) {
        qCWarning(lcDevice) << "No remote paths found";
        return std::nullopt;
    }

    return plist.first().id;
}

QString Device::toString() const
{
    QStringList l;
    l << QStringLiteral("Device{");
    l << QStringLiteral("seagateDeviceID:%1,").arg(seagateDeviceID);
    l << QStringLiteral("certificateCommonName:%1,").arg(certificateCommonName);
    l << QStringLiteral("friendlyName:%1,").arg(friendlyName());
    l << QStringLiteral("hostname:%1,").arg(hostname);
    l << QStringLiteral("origin:%1,").arg(DevHelpers::originToStr(origin));
    l << QStringLiteral("isStatic:%1,").arg(isStatic);
    for (const auto& p: paths) {
        l << p.toString();
    }
    l << QStringLiteral("}");
    return l.join(QStringLiteral(""));
}

void Device::setFriendlyName(const QString &name)
{
    _friendlyName = name;
}

Device Device::MakeStatic(const QString &url, const QString &name)
{
    Device dev;
    dev.certificateCommonName = name;
    dev.isStatic = true;
    dev.origin = DeviceOrigin::Static;
    DevicePath dp(url, DeviceType::Public, DeviceOrigin::Static, 0);
    dev.paths.append(dp);
    return dev;
}

QJsonDocument Device::toJson(const Device& dev)
{
    QJsonDocument doc;
    QJsonObject obj;
    obj[device_id_C] = dev.seagateDeviceID;
    obj[cert_common_name_C] = dev.certificateCommonName;
    obj[friendly_name_C] = dev.friendlyName();
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
    d.setFriendlyName(obj[friendly_name_C].toString());
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

QString DeviceHardwareInfo::toString() const
{
    QStringList l;
    l << QStringLiteral("DeviceHardwareInfo{");
    l << QStringLiteral("memory:%1,").arg(memory);
    l << QStringLiteral("processor_count:%1,").arg(processor_count);
    l << QStringLiteral("processor_type:%1").arg(processor_type);
    l << QStringLiteral("}");
    return l.join(QStringLiteral(""));
}

