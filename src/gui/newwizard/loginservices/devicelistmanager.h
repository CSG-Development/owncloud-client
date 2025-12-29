#pragma once

#include "device/devicetypes.h"

#include <QObject>

namespace CUR {

class MdnsClient;
class DeviceApi;

class DeviceListManager: public QObject
{
    Q_OBJECT

public:
    explicit DeviceListManager(QObject* parent = nullptr);

    void query_local();
    void query_remote(const QList<DeviceInfo>& devices);

    void query_local_status_queue();
    void query_local_about_queue();

    void query_ra_status_queue();
    void query_ra_about_queue();

    void query_local_status(const MdnsRecord& rec);
    void query_local_about(const MdnsRecord& rec);

    void query_ra_status(const DeviceInfo& rec);
    void query_ra_about(const DeviceInfo& rec);

    const QList<MdnsRecord>& mdns_recs() {return mdns_records;}
    const QList<DeviceInfo>& ra_recs() {return remote_devices;}

    static QList<Device> combine_lists(const QList<MdnsRecord>& mdns_records, const QList<DeviceInfo>& remote_devices);

signals:
    void local_finished();
    void ra_finished();

    void mdns_discovery_finished();

    void about_queue_local_finished();
    void status_queue_local_finished();

    void about_queue_ra_finished();
    void status_queue_ra_finished();

private:
    void append_local_about(const DeviceInfoAbout& info, const MdnsRecord& mrec);
    void append_local_status(const DeviceInfoStatus& info, const MdnsRecord& mrec);

    void append_ra_about(const DeviceInfoAbout& info, const DeviceInfo& mrec);
    void append_ra_status(const DeviceInfoStatus& info, const DeviceInfo& mrec);

    MdnsRecord* findLocalRec(const MdnsRecord& mrec);
    DeviceInfo* findRaRec(const DeviceInfo& rec);

    void on_about_local_finished(const DeviceInfoAbout& info, int code);
    void on_status_local_finished(const DeviceInfoStatus& info, int code);
    void on_about_ra_finished(const DeviceInfoAbout& info, int code);
    void on_status_ra_finished(const DeviceInfoStatus& info, int code);

private:
    MdnsClient* mdns = nullptr;
    DeviceApi* devApi = nullptr;

    QList<MdnsRecord> mdns_records;
    QList<MdnsRecord> mdns_records_queue;
    MdnsRecord processingRecord;

    QList<DeviceInfo> remote_devices;
    QList<DeviceInfo> remote_devices_queue;
    DeviceInfo processingRaRecord;
};

} // namespace CUR
