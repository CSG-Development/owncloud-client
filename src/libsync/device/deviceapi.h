#pragma once

#include "curatorlib.h"
#include "devicetypes.h"

#include <QObject>
#include <QNetworkAccessManager>
#include <QRestAccessManager>
#include <QNetworkRequestFactory>
#include <QJsonDocument>

namespace CUR {

class CURATORSYNC_EXPORT DeviceApi: public QObject
{
    Q_OBJECT

public:
    explicit DeviceApi(QObject* parent = nullptr);
    ~DeviceApi();

    void query_device_about(const QString &url);
    void query_device_status(const QString &url);

signals:
    void about_finished(const DeviceInfoAbout& info, int code);
    void status_finished(const DeviceInfoStatus& info, int code);

    void about_request_finished(std::optional<QJsonDocument>, int code);
    void status_request_finished(std::optional<QJsonDocument>, int code);

private:
    QNetworkAccessManager net_mgr;
    std::unique_ptr<QRestAccessManager> rest_mgr;
    std::unique_ptr<QNetworkRequestFactory> rest_factory;
};

} // namespace CUR
