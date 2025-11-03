#include "devicelistmanager.h"
#include "mdnsclient.h"
#include "deviceapi.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcDeviceManager, "device.manager", QtDebugMsg)

namespace CUR {

DeviceListManager::DeviceListManager(QObject *parent)
    : QObject(parent)
    , mdns(new MdnsClient(this))
    , devApi(new DeviceApi(this))
{
    connect(mdns, &MdnsClient::requestCompleted, this, [&] {
        mdns_records = mdns->records();
        mdns_records_queue = mdns_records;

        qCDebug(lcDeviceManager) << "mDNS records" << mdns_records.size();
        emit mdns_discovery_finished();
    });

    connect(this, &DeviceListManager::mdns_discovery_finished, this, [&] {
        if (mdns_records_queue.isEmpty()) {
            // No records from mDNS discovery, local finished
            emit local_finished();
            return;
        }
        query_local_about_queue();
    });

    connect(this, &DeviceListManager::about_queue_local_finished, this, [&] {
        mdns_records_queue = mdns_records;
        query_local_status_queue();
    });

    connect(this, &DeviceListManager::status_queue_local_finished, this, [&] {
        emit local_finished();
    });

    connect(this, &DeviceListManager::about_queue_ra_finished, this, [&] {
        remote_devices_queue = remote_devices;
        query_ra_status_queue();
    });

    connect(this, &DeviceListManager::status_queue_ra_finished, this, [&] {
        emit ra_finished();
    });

}

void DeviceListManager::query_local()
{
    qCDebug(lcDeviceManager) << "Query local devices";

    disconnect(devApi, &DeviceApi::about_finished, this, nullptr);
    disconnect(devApi, &DeviceApi::status_finished, this, nullptr);

    connect(devApi, &DeviceApi::about_finished, this, &DeviceListManager::on_about_local_finished);
    connect(devApi, &DeviceApi::status_finished, this, &DeviceListManager::on_status_local_finished);

    mdns->query();
}

void DeviceListManager::query_remote(const QList<DeviceInfo> &devices)
{
    qCDebug(lcDeviceManager) << "Query remote devices, count:" << devices.size();

    if (devices.isEmpty()) {
        emit ra_finished();
        return;
    }

    remote_devices = devices;

    disconnect(devApi, &DeviceApi::about_finished, this, nullptr);
    disconnect(devApi, &DeviceApi::status_finished, this, nullptr);

    connect(devApi, &DeviceApi::about_finished, this, &DeviceListManager::on_about_ra_finished);
    connect(devApi, &DeviceApi::status_finished, this, &DeviceListManager::on_status_ra_finished);

    remote_devices_queue = remote_devices;
    query_ra_about_queue();
}

void DeviceListManager::query_local_status_queue()
{
    if (mdns_records_queue.isEmpty()) {
        // No more records left
        emit status_queue_local_finished();
        qCWarning(lcDeviceManager) << "query_local_status_queue: empty queue";
        return;
    }
    processingRecord = mdns_records_queue.takeFirst();
    query_local_status(processingRecord);
}

void DeviceListManager::query_local_about_queue()
{
    if (mdns_records_queue.isEmpty()) {
        // No more records left
        emit about_queue_local_finished();
        qCWarning(lcDeviceManager) << "query_local_about_queue: empty queue";
        return;
    }

    processingRecord = mdns_records_queue.takeFirst();
    query_local_about(processingRecord);
}

void DeviceListManager::query_ra_status_queue()
{
    if (remote_devices_queue.isEmpty()) {
        emit status_queue_ra_finished();
        qCWarning(lcDeviceManager) << "query_ra_status_queue: empty queue";
        return;
    }

    processingRaRecord = remote_devices_queue.takeFirst();
    query_ra_status(processingRaRecord);
}

void DeviceListManager::query_ra_about_queue()
{
    qCDebug(lcDeviceManager) << "Query remote about queue";
    if (remote_devices_queue.isEmpty()) {
        emit about_queue_ra_finished();
        qCWarning(lcDeviceManager) << "query_ra_about_queue: empty queue";
        return;
    }

    processingRaRecord = remote_devices_queue.takeFirst();
    query_ra_about(processingRaRecord);
}

void DeviceListManager::query_local_status(const MdnsRecord &rec)
{
    auto url = normalizeUrl(rec.host, rec.port, false);
    qCDebug(lcDeviceManager) << "Query local status" << url;
    devApi->query_device_status(url);
}

void DeviceListManager::query_local_about(const MdnsRecord &rec)
{
    auto url = normalizeUrl(rec.host, rec.port, false);
    qCDebug(lcDeviceManager) << "Query local about" << url;
    devApi->query_device_about(url);
}

void DeviceListManager::query_ra_status(const DeviceInfo &rec)
{
    auto url = normalizeUrl(rec.host, rec.port, false);
    qCDebug(lcDeviceManager) << "Query remote status" << url;
    devApi->query_device_status(url);
}

void DeviceListManager::query_ra_about(const DeviceInfo &rec)
{
    auto url = normalizeUrl(rec.host, rec.port, false);
    qCDebug(lcDeviceManager) << "Query remote about" << url;
    devApi->query_device_about(url);
}

QList<Device> DeviceListManager::combine_lists(const QList<MdnsRecord>& mdns_records, const QList<DeviceInfo>& remote_devices)
{
    QList<Device> result;

    auto find_by_cert = [&](const QString& certName) -> Device* {
        for (int i = 0; i < result.size(); i++) {
            if (certName == result[i].certificateCommonName) {
                return &(result[i]);
            }
        }
        return nullptr;
    };

    auto find_in_paths = [&](Device* dev, const QString& host, int port) -> DevicePath* {
        for (int i = 0; i < dev->paths.size(); i++) {
            if (dev->paths[i].address == host && dev->paths[i].port == port)
                return &(dev->paths[i]);
        }
        return nullptr;
    };

    auto append_local_path = [&](Device* dev, const MdnsRecord& rec) {
        if (rec.status.state != QStringLiteral("ready") || !rec.status.oobe_done)
            return;

        DevicePath dev_path;
        dev_path.address = rec.host;
        dev_path.port = rec.port;
        dev_path.deviceType = DeviceType::Local;
        dev->paths.append(dev_path);
    };

    auto append_remote_path = [&](Device* dev, const DeviceInfo& rec) {
        if (rec.status.state != QStringLiteral("ready") || !rec.status.oobe_done)
            return;

        DevicePath dev_path;
        dev_path.address = rec.host;
        dev_path.port = rec.port;
        dev_path.deviceType = DeviceType::Remote;
        dev->paths.append(dev_path);
    };

    auto append_local_device = [&](const MdnsRecord& rec) {
        if (rec.status.state != QStringLiteral("ready") || !rec.status.oobe_done)
            return;

        Device dev;
        dev.certificateCommonName = rec.about.certificate_common_name;
        dev.hostname = rec.about.hostname;
        DevicePath dev_path;
        dev_path.address = rec.host;
        dev_path.port = rec.port;
        dev_path.deviceType = DeviceType::Local;
        dev.paths.append(dev_path);
        result.append(dev);
    };

    auto append_remote_device = [&](const DeviceInfo& rec) {
        if (rec.status.state != QStringLiteral("ready") || !rec.status.oobe_done)
            return;

        Device dev;
        dev.certificateCommonName = rec.about.certificate_common_name;
        dev.hostname = rec.about.hostname;
        DevicePath dev_path;
        dev_path.address = rec.host;
        dev_path.port = rec.port;
        dev_path.deviceType = rec.deviceType;
        dev.paths.append(dev_path);
        result.append(dev);
    };

    for (const auto& item: mdns_records) {
        if (auto d = find_by_cert(item.about.certificate_common_name)) {
            if (auto p = find_in_paths(d, item.host, item.port)) {
                qCDebug(lcDeviceManager) << "Already in list" << item.host << item.port;
            }
            else {
                append_local_path(d, item);
            }
        }
        else {
            append_local_device(item);
        }
    }

    for (const auto& item: remote_devices) {
        if (auto d = find_by_cert(item.about.certificate_common_name)) {
            if (auto p = find_in_paths(d, item.host, item.port)) {
                qCDebug(lcDeviceManager) << "Already in list" << item.host << item.port;
            }
            else {
                append_remote_path(d, item);
            }
        }
        else {
            append_remote_device(item);
        }
    }

    return result;
}

void DeviceListManager::append_local_about(const DeviceInfoAbout &info, const MdnsRecord& mrec)
{
    if (auto rec = findLocalRec(mrec)) {
        rec->about = info;
    }
}

void DeviceListManager::append_local_status(const DeviceInfoStatus &info, const MdnsRecord& mrec)
{
    if (auto rec = findLocalRec(mrec)) {
        rec->status = info;
    }
}

void DeviceListManager::append_ra_about(const DeviceInfoAbout &info, const DeviceInfo& mrec)
{
    if (auto rec = findRaRec(mrec)) {
        rec->about = info;
    }
}

void DeviceListManager::append_ra_status(const DeviceInfoStatus &info, const DeviceInfo& mrec)
{
    if (auto rec = findRaRec(mrec)) {
        rec->status = info;
    }
}

MdnsRecord *DeviceListManager::findLocalRec(const MdnsRecord &mrec)
{
    for (int i = 0; i < mdns_records.size(); i++) {
        if (mdns_records[i].host == mrec.host && mdns_records[i].port == mrec.port)
            return &mdns_records[i];
    }
    return nullptr;
}

DeviceInfo *DeviceListManager::findRaRec(const DeviceInfo &rec)
{
    for (int i = 0; i < remote_devices.size(); i++) {
        if (remote_devices[i].host == rec.host && remote_devices[i].port == rec.port)
            return &remote_devices[i];
    }
    return nullptr;
}

void DeviceListManager::on_about_local_finished(const DeviceInfoAbout &info, int code)
{
    qCDebug(lcDeviceManager) << "about_local_finished" << info << "code" << code;
    if (code == 200) {
        append_local_about(info, processingRecord);
    }
    else {
    }

    if (mdns_records_queue.isEmpty()) {
        // No more records left
        emit about_queue_local_finished();
        return;
    }

    // Query next record
    processingRecord = mdns_records_queue.takeFirst();
    query_local_about(processingRecord);
}

void DeviceListManager::on_status_local_finished(const DeviceInfoStatus &info, int code)
{
    qCDebug(lcDeviceManager) << "status_local_finished" << info << "code" << code;
    if (code == 200) {
        append_local_status(info, processingRecord);
    }
    else {
    }

    if (mdns_records_queue.isEmpty()) {
        // No more records left
        emit status_queue_local_finished();
        return;
    }

    // Query next record
    processingRecord = mdns_records_queue.takeFirst();
    query_local_status(processingRecord);
}

void DeviceListManager::on_about_ra_finished(const DeviceInfoAbout &info, int code)
{
    qCDebug(lcDeviceManager) << "about_ra_finished" << info << "code" << code;
    if (code == 200) {
        append_ra_about(info, processingRaRecord);
    }
    else {
    }

    if (remote_devices_queue.isEmpty()) {
        // No more records left
        emit about_queue_ra_finished();
        return;
    }

    // Query next record
    processingRaRecord = remote_devices_queue.takeFirst();
    query_ra_about(processingRaRecord);
}

void DeviceListManager::on_status_ra_finished(const DeviceInfoStatus &info, int code)
{
    qCDebug(lcDeviceManager) << "status_ra_finished" << info << "code" << code;
    if (code == 200) {
        append_ra_status(info, processingRaRecord);
    }
    else {
    }

    if (remote_devices_queue.isEmpty()) {
        // No more records left
        emit status_queue_ra_finished();
        return;
    }

    // Query next record
    processingRaRecord = remote_devices_queue.takeFirst();
    query_ra_status(processingRaRecord);
}

} // namespace CUR
