#include "devicetypes.h"
#include "devicelogging.h"

#include <QMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLoggingCategory>
#include <QSet>

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
const auto remote_paths_fetched_at_utc_C = QStringLiteral("remote_paths_fetched_at_utc");
constexpr int DefaultHttpsPort = 443;

QString remotePathKey(const DevicePath& path)
{
    return QStringLiteral("%1|%2|%3")
        .arg(path.address, QString::number(path.port), DevHelpers::devTypeToStr(path.deviceType));
}

QSet<QString> remotePathKeySet(const QList<DevicePath>& paths)
{
    QSet<QString> keys;
    for (const auto& path : paths) {
        keys.insert(remotePathKey(path));
    }
    return keys;
}
}

Q_LOGGING_CATEGORY(lcDevice, "device", QtDebugMsg)
Q_LOGGING_CATEGORY(lcDeviceData, "device.data", QtWarningMsg)
Q_LOGGING_CATEGORY(lcRaPerformance, "ra.performance", QtInfoMsg)

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
    const auto address = val[address_C].toString();
    const auto rawPort = val[port_C].toInt();
    const auto port = rawPort > 0 ? rawPort : DefaultHttpsPort;
    const auto rawDeviceType = val[deviceType_C].toString();
    const auto deviceType = DevHelpers::strToDevType(rawDeviceType);
    if (address.isEmpty() || deviceType == DeviceType::Unknown) {
        qCWarning(lcDeviceData).noquote()
            << "device_cache invalid_path"
            << QStringLiteral("{address:%1,port:%2,type:%3}")
                   .arg(address, QString::number(rawPort), rawDeviceType);
    }

    DevicePath dp(
        address,
        deviceType,
        DevHelpers::strToDevOrigin(val[deviceOrigin_C].toString()),
        port);
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
    l << QStringLiteral("id:%1,").arg(id.toString());
    l << QStringLiteral("%1,").arg(about.toString());
    l << status.toString();
    l << QStringLiteral("}");
    return l.join(QStringLiteral(""));
}

QString DevicePath::toStringShort() const
{
    QStringList l;
    l << QStringLiteral("path{");
    l << QStringLiteral("addr:%1:%2,").arg(address).arg(port);
    l << QStringLiteral("type:%1,").arg(DevHelpers::devTypeToStr(deviceType));
    l << QStringLiteral("origin:%1,").arg(DevHelpers::originToStr(origin));
    l << QStringLiteral("id:%1,").arg(id.toString());
    l << QStringLiteral("%1,").arg(about.toStringShort());
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

    QList<DevicePath> dev_paths;
    for (const auto& p: dev.paths) {
        if (p.deviceType != DeviceType::Unknown && p.status.oobe_done) {
            dev_paths.append(p);
        }
    }

    // No online paths found
    if (dev_paths.isEmpty())
        return dev.paths.first().id;

    if (dev_paths.size() == 1)
        return dev_paths.first().id;

    std::stable_sort(dev_paths.begin(), dev_paths.end(), [&](const DevicePath& a, const DevicePath& b) {
        return DevicePath::pathPriority(a.deviceType) > DevicePath::pathPriority(b.deviceType);
    });

    return dev_paths.first().id;
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

QList<DevicePath> Device::remotePaths() const
{
    QList<DevicePath> ret;
    for (const auto& path: std::as_const(paths)) {
        if (path.origin == DeviceOrigin::Remote) {
            ret.append(path);
        }
    }
    return ret;
}

QList<DevicePath> Device::nonRemotePaths(const QList<DevicePath>& paths)
{
    QList<DevicePath> ret;
    for (const auto& path : paths) {
        if (path.origin != DeviceOrigin::Remote) {
            ret.append(path);
        }
    }
    return ret;
}

QList<DevicePath> Device::normalizeRemotePaths(const QList<DevicePath>& paths)
{
    QList<DevicePath> ret(paths);
    for (auto& path : ret) {
        path.origin = DeviceOrigin::Remote;
    }
    return ret;
}

QList<DevicePath> Device::replaceRemotePaths(const QList<DevicePath>& existingPaths, const QList<DevicePath>& remotePaths)
{
    return DeviceList::mergePaths(nonRemotePaths(existingPaths), normalizeRemotePaths(remotePaths));
}

void Device::updateRemotePathCache(const QList<DevicePath>& remotePaths)
{
    updateRemotePathCache(remotePaths, QDateTime::currentDateTimeUtc());
}

void Device::updateRemotePathCache(const QList<DevicePath>& remotePaths, const QDateTime& fetchedAtUtc)
{
    paths = replaceRemotePaths(paths, remotePaths);
    remotePathsFetchedAtUtc = fetchedAtUtc.isValid() ? fetchedAtUtc.toUTC() : QDateTime::currentDateTimeUtc();
}

bool Device::hasSameRemotePaths(const QList<DevicePath>& paths) const
{
    return remotePathKeySet(remotePaths()) == remotePathKeySet(normalizeRemotePaths(paths));
}

bool Device::hasRemotePathCache() const
{
    return !remotePaths().isEmpty() && remotePathsFetchedAtUtc.isValid();
}

bool Device::isRemotePathCacheExpired(int ttlSeconds) const
{
    if (!hasRemotePathCache()) {
        return true;
    }

    return remotePathsFetchedAtUtc.secsTo(QDateTime::currentDateTimeUtc()) > ttlSeconds;
}

QString Device::toString() const
{
    QStringList l;
    l << QStringLiteral("Device{");
    l << QStringLiteral("seagateDeviceID:%1,").arg(seagateDeviceID);
    l << QStringLiteral("certificateCommonName:%1,").arg(certificateCommonName);
    l << QStringLiteral("friendlyName:%1,").arg(friendlyName());
    l << QStringLiteral("hostname:%1,").arg(hostname);
    l << QStringLiteral("isStatic:%1,").arg(isStatic);
    l << QStringLiteral("remotePathsFetchedAtUtc:%1,").arg(remotePathsFetchedAtUtc.toString(Qt::ISODate));
    for (const auto& p: paths) {
        l << p.toString();
    }
    l << QStringLiteral("}");
    return l.join(QStringLiteral(""));
}

QString Device::toStringShort() const
{
    QStringList l;
    l << QStringLiteral("Device{");
    l << QStringLiteral("id:%1,").arg(seagateDeviceID);
    l << QStringLiteral("certCN:%1,").arg(certificateCommonName);
    l << QStringLiteral("friendlyName:%1,").arg(friendlyName());
    l << QStringLiteral("hostname:%1,").arg(hostname);
    l << QStringLiteral("static:%1,").arg(isStatic);
    l << QStringLiteral("remoteCacheAt:%1,").arg(remotePathsFetchedAtUtc.toString(Qt::ISODate));
    for (const auto& p: paths) {
        l << p.toStringShort();
    }
    l << QStringLiteral("}");
    return l.join(QStringLiteral(""));
}

Device Device::MakeStatic(const QString &url, const QString &name)
{
    Device dev;
    dev.certificateCommonName = name;
    dev.setFriendlyName(name);
    dev.isStatic = true;
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
    if (dev.remotePathsFetchedAtUtc.isValid()) {
        obj[remote_paths_fetched_at_utc_C] = dev.remotePathsFetchedAtUtc.toUTC().toString(Qt::ISODate);
    }
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
    const auto rawRemotePathsFetchedAtUtc = obj[remote_paths_fetched_at_utc_C].toString();
    if (!rawRemotePathsFetchedAtUtc.isEmpty()) {
        const auto parsed = QDateTime::fromString(rawRemotePathsFetchedAtUtc, Qt::ISODate);
        if (parsed.isValid()) {
            d.remotePathsFetchedAtUtc = parsed.toUTC();
        } else {
            qCWarning(lcDeviceData).noquote()
                << "device_cache invalid_remote_cache_timestamp"
                << QStringLiteral("{value:%1,cn:%2,id:%3}")
                       .arg(rawRemotePathsFetchedAtUtc, d.certificateCommonName, d.seagateDeviceID);
        }
    }

    if (d.seagateDeviceID.isEmpty() && !d.remotePaths().isEmpty()) {
        qCWarning(lcDeviceData).noquote()
            << "device_cache inconsistent_remote_state"
            << QStringLiteral("{cn:%1,remotePathCount:%2}")
                   .arg(d.certificateCommonName, QString::number(d.remotePaths().size()));
    }

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


void DeviceList::addDevice(const Device &d)
{
    auto it = std::find_if(dev_list.begin(), dev_list.end(), [&d](const Device& item) {
        return item.certificateCommonName == d.certificateCommonName;
    });

    if (it != dev_list.end()) {
        if (!it->seagateDeviceID.isEmpty() && !d.seagateDeviceID.isEmpty() && it->seagateDeviceID != d.seagateDeviceID) {
            qCWarning(lcDeviceData).noquote()
                << "device_list_add cn_id_conflict"
                << QStringLiteral("{cn:%1,firstId:%2,secondId:%3}")
                       .arg(d.certificateCommonName, it->seagateDeviceID, d.seagateDeviceID);
        }
        if (it->seagateDeviceID.isEmpty() && !d.seagateDeviceID.isEmpty()) {
            it->seagateDeviceID = d.seagateDeviceID;
        }
        if (it->friendlyName().isEmpty() && !d.friendlyName().isEmpty()) {
            it->setFriendlyName(d.friendlyName());
        }
        if (it->hostname.isEmpty() && !d.hostname.isEmpty()) {
            it->hostname = d.hostname;
        }
        if (!it->remotePathsFetchedAtUtc.isValid() && d.remotePathsFetchedAtUtc.isValid()) {
            it->remotePathsFetchedAtUtc = d.remotePathsFetchedAtUtc;
        }
        (*it).paths = mergePaths((*it).paths, d.paths);
    }
    else {
        dev_list.append(d);
    }
}

void DeviceList::setDevices(const QList<Device> &devList)
{
    // QMap<QString,Device> map;
    // for (auto it = devList.cbegin(); it != devList.cend(); ++it) {
    //     map[it->seagateDeviceID] = *it;
    // }

    // QList<Device> tmp(map.values());
    QList<Device> tmp(devList);
    qSwap(tmp, dev_list);
}

void DeviceList::clear()
{
    dev_list.clear();
}

void DeviceList::sort_by_static()
{
    std::stable_sort(dev_list.begin(), dev_list.end(), [](const Device& a, const Device& b) {
        return a.isStatic > b.isStatic;
    });
}

std::optional<Device> DeviceList::find_by_cn(const QString &cn) const
{
    const auto it = std::find_if(dev_list.cbegin(), dev_list.cend(), [cn](const Device& d) {
        return d.certificateCommonName == cn;
    });
    if (it != dev_list.cend())
        return *it;
    return std::nullopt;
}

QList<DevicePath> DeviceList::mergePaths(const QList<DevicePath> &path_1, const QList<DevicePath> &path_2)
{
    QMap<std::pair<QString, int>, DevicePath> uniqueMap;

    auto insertOrUpdate = [&](const DevicePath &path) {
        auto key = std::make_pair(path.address, path.port);

        if (!uniqueMap.contains(key)) {
            uniqueMap.insert(key, path);
        } else {
            if (uniqueMap[key].deviceType != path.deviceType) {
                qCWarning(lcDeviceData).noquote()
                    << "merge_paths endpoint_type_conflict"
                    << QStringLiteral("{endpoint:%1:%2,firstType:%3,secondType:%4}")
                           .arg(path.address, QString::number(path.port),
                               DevHelpers::devTypeToStr(uniqueMap[key].deviceType), DevHelpers::devTypeToStr(path.deviceType));
            }
            if (path.origin == DeviceOrigin::MDNS && uniqueMap[key].origin != DeviceOrigin::MDNS) {
                uniqueMap[key] = path;
            }
        }
    };

    for (const auto &path : path_1) {
        insertOrUpdate(path);
    }

    for (const auto &path : path_2) {
        insertOrUpdate(path);
    }

    return uniqueMap.values();
}

void Device::setFriendlyName(const QString &fn)
{
    _friendlyName = fn;
}
