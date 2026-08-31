#include "deviceaggregator.h"
#include "devicedefines.h"
#include "devicelogging.h"
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
    QWriteLocker locker(&lock_);
    mergedPath_.clear();
}

DeviceList DeviceAggregator::mergeDevices(const DeviceList& dev_1, const DeviceList& dev_2)
{
    QMap<QString, Device> mergedMap;

    qCDebug(lcDeviceAggregator) << "mergeDevices" << dev_1.devices() << dev_2.devices();

    auto addToMap = [&mergedMap](const Device &device) {
        if (!mergedMap.contains(device.certificateCommonName)) {
            qCDebug(lcDeviceAggregator) << "Insert" << device.certificateCommonName << device.friendlyName();
            mergedMap.insert(device.certificateCommonName, device);
        } else {
            Device &existingDevice = mergedMap[device.certificateCommonName];

            qCDebug(lcDeviceAggregator) << "Exist" << device.certificateCommonName;
            qCDebug(lcDeviceAggregator) << "existing" << existingDevice.friendlyName() << "new" << device.friendlyName();

            if (!existingDevice.seagateDeviceID.isEmpty()
                && !device.seagateDeviceID.isEmpty()
                && existingDevice.seagateDeviceID != device.seagateDeviceID) {
                qCWarning(lcDeviceData).noquote()
                    << "merge_devices cn_id_conflict"
                    << QStringLiteral("{cn:%1,firstId:%2,secondId:%3}")
                           .arg(device.certificateCommonName, existingDevice.seagateDeviceID, device.seagateDeviceID);
            }

            // update DeviceID if empty
            if (existingDevice.seagateDeviceID.isEmpty() && !device.seagateDeviceID.isEmpty()) {
                existingDevice.seagateDeviceID = device.seagateDeviceID;
            }
            if (existingDevice.friendlyName().isEmpty() && !device.friendlyName().isEmpty()) {
                existingDevice.setFriendlyName(device.friendlyName());
            }
            if (existingDevice.hostname.isEmpty() && !device.hostname.isEmpty()) {
                existingDevice.hostname = device.hostname;
            }
            if (!existingDevice.remotePathsFetchedAtUtc.isValid() && device.remotePathsFetchedAtUtc.isValid()) {
                existingDevice.remotePathsFetchedAtUtc = device.remotePathsFetchedAtUtc;
            }

            // paths join
            for (const auto &newPath : device.paths) {
                bool foundMatch = false;

                for (int i = 0; i < existingDevice.paths.size(); ++i) {
                    DevicePath &existingPath = existingDevice.paths[i];

                    if (existingPath.address == newPath.address && existingPath.port == newPath.port) {
                        foundMatch = true;

                        if (existingPath.deviceType != newPath.deviceType) {
                            qCWarning(lcDeviceData).noquote()
                                << "merge_devices endpoint_type_conflict"
                                << QStringLiteral("{cn:%1,endpoint:%2:%3,firstType:%4,secondType:%5}")
                                       .arg(device.certificateCommonName, newPath.address, QString::number(newPath.port),
                                           DevHelpers::devTypeToStr(existingPath.deviceType), DevHelpers::devTypeToStr(newPath.deviceType));
                        }

                        // Keep MDNS origin
                        if (newPath.origin == DeviceOrigin::MDNS && existingPath.origin != DeviceOrigin::MDNS) {
                            existingPath.origin = newPath.origin;
                        }
                        break;
                    }
                }

                if (!foundMatch) {
                    existingDevice.paths.append(newPath);
                    qCDebug(lcDeviceAggregator) << "path append" << newPath.toStringShort();
                }
                else {
                    qCDebug(lcDeviceAggregator) << "path exist" << newPath.toStringShort();
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
        if (cn.isEmpty()) {
            qCWarning(lcDeviceData).noquote()
                << "build_devices invalid_record missing_cn"
                << QStringLiteral("{endpoint:%1:%2,origin:%3}")
                       .arg(record.address, QString::number(record.port), DevHelpers::originToStr(record.origin));
        }

        if (!deviceMap.contains(cn)) {
            Device newDevice;
            newDevice.certificateCommonName = cn;
            newDevice.hostname = record.about.hostname;
            if (!record.friendlyName.isEmpty()) {
                newDevice.setFriendlyName(record.friendlyName);
            }
            else if (!cn.isEmpty()) {
                newDevice.setFriendlyName(cn);
            }
            else {
                qCDebug(lcDeviceAggregator) << "failed to get friendlyName";
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
                if (device.friendlyName().isEmpty() && !record.friendlyName.isEmpty()) {
                    device.setFriendlyName(record.friendlyName);
                    qCDebug(lcDeviceAggregator) << "updated empty friendlyName to" << device.friendlyName();
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
    for (const auto& it: std::as_const(mergedPath_)) {
        const QString key = it.address + QStringLiteral(":") + QString::number(it.port);
        existingPaths.insert(key, it);
    }

    for (const auto& it: std::as_const(devs)) {
        const QString key = it.address + QStringLiteral(":") + QString::number(it.port);
        if (existingPaths.contains(key)) {
            const auto& existingPath = existingPaths[key];
            if (!existingPath.about.certificate_common_name.isEmpty()
                && !it.about.certificate_common_name.isEmpty()
                && existingPath.about.certificate_common_name != it.about.certificate_common_name) {
                qCWarning(lcDeviceData).noquote()
                    << "add_paths endpoint_cn_conflict"
                    << QStringLiteral("{endpoint:%1,firstCn:%2,secondCn:%3}")
                           .arg(key, existingPath.about.certificate_common_name, it.about.certificate_common_name);
            }
            qCDebug(lcDeviceAggregator) << "exist" << key;
        }
        else {
            qCDebug(lcDeviceAggregator) << "added" << key;
            existingPaths.insert(key, it);
        }
    }

    {
        QWriteLocker locker(&lock_);
        mergedPath_ = existingPaths.values();
    }

    // !! emit outside lock scope
    emit listUpdated();

}

void DeviceAggregator::clear_paths()
{
    QWriteLocker locker(&lock_);
    mergedPath_.clear();
}
