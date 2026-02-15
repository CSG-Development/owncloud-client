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

void DeviceAggregator::clearAll()
{
    {
        QWriteLocker locker(&lock_);
        mergedList_.clear();
    }
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

DeviceList DeviceAggregator::mergeDevices(const DeviceList& dev_1, const DeviceList& dev_2)
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

    for (const auto &d : dev_1.devices())
        addToMap(d);
    for (const auto &d : dev_2.devices())
        addToMap(d);

    DeviceList dl;
    dl.setDevices(mergedMap.values());

    return dl;
}

DeviceList DeviceAggregator::build_devices(const QList<DevicePath> &records)
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

    DeviceList dl;
    dl.setDevices(deviceMap.values());
    return dl;
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

