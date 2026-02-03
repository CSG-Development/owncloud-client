#include "deviceaggregator.h"
#include "devicedefines.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcDeviceAggregator, "device.aggregator", QtDebugMsg)

namespace {
int getPriority(DeviceOrigin origin)
{
    switch (origin) {
    case DeviceOrigin::Static: return 100;
    case DeviceOrigin::MDNS:   return 50;
    case DeviceOrigin::Remote: return 10;
    default:                   return 0;
    }
}
}

DeviceAggregator::DeviceAggregator(QObject *parent)
    : QObject(parent)
{
}

QList<DevicePath> DeviceAggregator::getDevicePaths() const
{
    QReadLocker locker(&lock_);
    return mergedList_;
}

void DeviceAggregator::updateSource(DeviceOrigin origin, const QList<DevicePath> &newDevices)
{
    {
        QWriteLocker locker(&lock_);
        sourceStorage_[origin] = newDevices;
        rebuildInternal();
    }

    // !! emit outside lock scope
    emit listUpdated();
}

void DeviceAggregator::clearAll()
{
    {
        QWriteLocker locker(&lock_);
        sourceStorage_.clear();
        mergedList_.clear();
    }
}

void DeviceAggregator::rebuildInternal()
{
    QMap<QString, DevicePath> uniqueMap;

    for (auto it = sourceStorage_.begin(); it != sourceStorage_.end(); ++it) {

        const DeviceOrigin currentOrigin = it.key();
        const QList<DevicePath>& devices = it.value();

        qCDebug(lcDeviceAggregator) << "srcStorage" << DevHelpers::originToStr(currentOrigin) << devices;

        for (const auto& newDev : devices) {
            QString key = QStringLiteral("%1:%2").arg(newDev.address).arg(newDev.port);

            if (!uniqueMap.contains(key) ||
                getPriority(currentOrigin) > getPriority(uniqueMap[key].origin)) {
                uniqueMap.insert(key, newDev);
            }
        }
    }

    mergedList_ = uniqueMap.values();
}

void DeviceAggregator::merge(Device &target, const QList<DevicePath> &path_sources)
{
    QHash<QString, int> existingPathIndices;
    for (int i = 0; i < target.paths.size(); ++i) {
        QString key = target.paths[i].address + QStringLiteral(":") + QString::number(target.paths[i].port);
        existingPathIndices[key] = i;
    }

    for (const auto &newPath : path_sources) {
        if (target.certificateCommonName != newPath.about.certificate_common_name) {
            continue;
        }

        QString key = newPath.address + QStringLiteral(":") + QString::number(newPath.port);

        if (existingPathIndices.contains(key)) {
            int index = existingPathIndices[key];
            if (getPriority(newPath.origin) > getPriority(target.paths[index].origin)) {
                target.paths[index] = newPath;
            }
        } else {
            target.paths.append(newPath);
            existingPathIndices[key] = target.paths.size() - 1;
        }
    }
}

QList<Device> DeviceAggregator::mergeDevices(const QList<Device> &dev_1, const QList<Device> &dev_2)
{
    QHash<QString, Device> mergedMap;

    auto addToMap = [&mergedMap](const Device &device) {
        if (!mergedMap.contains(device.seagateDeviceID)) {
            mergedMap.insert(device.seagateDeviceID, device);
        } else {
            Device &existingDevice = mergedMap[device.seagateDeviceID];

            // update DeviceID if empty
            if (existingDevice.seagateDeviceID.isEmpty() && !device.seagateDeviceID.isEmpty()) {
                existingDevice.seagateDeviceID = device.seagateDeviceID;
            }

            // paths join
            for (const auto &newPath : device.paths) {
                bool foundMatch = false;

                for (int i = 0; i < existingDevice.paths.size(); ++i) {
                    DevicePath &existingPath = existingDevice.paths[i];

                    if (existingPath.address == newPath.address && existingPath.port == newPath.port) {
                        foundMatch = true;

                        // Keep MDNS origin
                        if (newPath.origin == DeviceOrigin::MDNS && existingPath.origin != DeviceOrigin::MDNS) {
                            existingPath.origin = newPath.origin;
                        }
                        break;
                    }
                }

                if (!foundMatch) {
                    existingDevice.paths.append(newPath);
                }
            }
        }
    };

    for (const auto &d : dev_1)
        addToMap(d);
    for (const auto &d : dev_2)
        addToMap(d);

    return mergedMap.values();
}

QList<DevicePath> DeviceAggregator::mergePaths(const QList<DevicePath> &path_1, const QList<DevicePath> &path_2)
{
    QMap<std::pair<QString, int>, DevicePath> uniqueMap;

    auto insertOrUpdate = [&](const DevicePath &path) {
        auto key = std::make_pair(path.address, path.port);

        if (!uniqueMap.contains(key)) {
            uniqueMap.insert(key, path);
        } else {
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

QList<Device> DeviceAggregator::build_devices(const QList<DevicePath> &records)
{
    QHash<QString, Device> deviceMap;

    qCDebug(lcDeviceAggregator) << records;

    for (const auto& record : records) {
        const QString& cn = record.about.certificate_common_name;

        if (!deviceMap.contains(cn)) {
            Device newDevice;
            newDevice.certificateCommonName = cn;
            if (!record.friendlyName.isEmpty()) {
                newDevice.friendlyName = record.friendlyName;
            }
            if (newDevice.friendlyName.isEmpty() && !record.about.hostname.isEmpty()) {
                newDevice.friendlyName = record.about.hostname;
            }

            deviceMap.insert(cn, newDevice);
            qCDebug(lcDeviceAggregator) << "insert new device" << newDevice;
        }

        Device& device = deviceMap[cn];
        bool foundDuplicate = false;

        for (int i = 0; i < device.paths.size(); ++i) {
            if (device.paths[i].address == record.address && device.paths[i].port == record.port) {

                foundDuplicate = true;
                if (getPriority(record.origin) > getPriority(device.paths[i].origin)) {
                    device.paths[i] = record;
                    qCDebug(lcDeviceAggregator) << "updated by priority" << record;
                }
                if (device.friendlyName.isEmpty() && record.friendlyName.isEmpty()) {
                    device.friendlyName = record.friendlyName;
                    qCDebug(lcDeviceAggregator) << "updated empty friendlyName to" << device.friendlyName;
                }

                break;
            }
        }

        if (!foundDuplicate) {
            device.paths.append(record);
        }
    }

    return deviceMap.values();
}

void DeviceAggregator::add_paths(const QList<DevicePath> &devs)
{
    QHash<QString, DevicePath> existingPaths;
    for (const auto& it: std::as_const(paths_)) {
        const QString key = it.address + QStringLiteral(":") + QString::number(it.port);
        existingPaths.insert(key, it);
    }

    for (const auto& it: std::as_const(devs)) {
        const QString key = it.address + QStringLiteral(":") + QString::number(it.port);
        if (existingPaths.contains(key)) {
            qCDebug(lcDeviceAggregator) << "exist" << key;
        }
        else {
            qCDebug(lcDeviceAggregator) << "added" << key;
            existingPaths.insert(key, it);
        }
    }

    {
        QWriteLocker locker(&lock_);
        paths_ = existingPaths.values();
    }

    // !! emit outside lock scope
    emit listUpdated();

}

void DeviceAggregator::clear_paths()
{
    QWriteLocker locker(&lock_);
    paths_.clear();
}

