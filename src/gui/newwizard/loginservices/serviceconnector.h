#pragma once

#include "devicetypes.h"

#include <QObject>
#include <QNetworkAccessManager>
#include <QRestAccessManager>
#include <QJsonDocument>
#include <QNetworkRequestFactory>
#include <QDateTime>

namespace CUR {

class ServiceConnector: public QObject
{
    Q_OBJECT

public:
    explicit ServiceConnector(QObject* parent = nullptr);
    ~ServiceConnector();

    void setRaCert();

    void start_query(const QString& email);

    void query_initiate(const QString& email);
    void query_refresh(const QString &refresh_token);
    void query_token(const QString& code);
    void query_devices(const QString &access_token);
    void query_device_info(const QString &access_token, const QString& deviceId);

    [[nodiscard]] const QList<DeviceInfo>& deviceList() const {return devInfoList;}

signals:
    void initiate_finished(std::optional<QJsonDocument>, int code);
    void refresh_finished(std::optional<QJsonDocument>, int code);
    void token_finished(std::optional<QJsonDocument>, int code);
    void devices_finished(std::optional<QJsonDocument>, int code);
    void device_info_finished(std::optional<QJsonDocument>, int code);

    void fetch_devices();
    void fetch_devices_finished();
    void code_requested();

    void error_code(int code);

    void request_finished(std::optional<QJsonDocument>, int code, const QString& url);

private:
    void saveRefreshToken();
    void loadRefreshToken();
    Device* findDevice(const QString& devId);

    void parseTokenReply(const QJsonDocument& doc);
    void parseDeviceInfoReply(const QJsonDocument& doc);
    void addDevice(const QJsonValue& val);
    void prepareDevList();

private:
    QNetworkAccessManager net_mgr;
    std::unique_ptr<QRestAccessManager> rest_mgr;
    std::unique_ptr<QNetworkRequestFactory> rest_factory;

    QSslConfiguration ssl_config;
    QSslCertificate cert;
    QDateTime accessTokenExpireTime;

    QString refreshToken;
    QString accessToken;
    QString referenceCode;
    QString tokenType;

    QList<Device> devices;
    QList<QString> devIdsQueue;
    QList<DeviceInfo> devInfoList;
};

} // namespace CUR
