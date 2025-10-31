#pragma once

#include "devicetypes.h"

#include <QObject>
#include <QNetworkAccessManager>
#include <QRestAccessManager>
#include <QNetworkRequestFactory>
#include <QDnsLookup>
#include <QJsonDocument>

namespace CUR {

class MdnsClient: public QObject
{
    Q_OBJECT

public:
    explicit MdnsClient(QObject* parent = nullptr);
    ~MdnsClient();

    void query();

    [[nodiscard]] const QList<DeviceInfo>& records() {return records_;}

signals:
    void requestCompleted();
    void device_info_finished(std::optional<QJsonDocument>, int code);

private:
    void query_device_info();
    void parse_device_about(const QJsonDocument& doc, LocalDeviceInfo &info);

private:
    QDnsLookup* dns = nullptr;
    QList<DeviceInfo> records_;

    QNetworkAccessManager net_mgr;
    std::unique_ptr<QRestAccessManager> rest_mgr;
    std::unique_ptr<QNetworkRequestFactory> rest_factory;
    QList<DeviceInfo> deviceRequestQueue;
    DeviceInfo processingDevice;
};

} // namespace CUR
