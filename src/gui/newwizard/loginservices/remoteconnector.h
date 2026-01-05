#pragma once

#include "device/devicetypes.h"

#include <QObject>
#include <QNetworkAccessManager>
#include <QRestAccessManager>
#include <QJsonDocument>
#include <QNetworkRequestFactory>
#include <QDateTime>

namespace CUR {

enum class RemoteRequest {
    Initiate,
    Refresh,
    Token,
    DeviceList,
    DeviceInfo
};

class RemoteConnector: public QObject
{
    Q_OBJECT

public:
    explicit RemoteConnector(QObject* parent = nullptr);
    ~RemoteConnector();

    void setRaCert();

    void start_query(const QString& email);

    void query_initiate(const QString& email);
    void query_refresh(const QString &refresh_token);
    void query_token(const QString& code);
    void query_devices_list(const QString &access_token);
    void query_device_info(const QString &access_token, const QString& deviceId);

    [[nodiscard]] const QList<DeviceInfo>& deviceList() const {return devInfoList;}

    static QString RemoteRequestToStr(RemoteRequest req);

    void clearTokens();

signals:
    void initiate_finished(std::optional<QJsonDocument>, int code);
    void refresh_finished(std::optional<QJsonDocument>, int code);
    void token_finished(std::optional<QJsonDocument>, int code);
    void devices_list_finished(std::optional<QJsonDocument>, int code);
    void device_info_finished(std::optional<QJsonDocument>, int code);

    void fetch_devices();
    void fetch_devices_finished();
    void code_requested();
    void token_success();

    void error_occured(RemoteRequest request, int code, const QString& message);

    void request_finished(std::optional<QJsonDocument>, int code, const QString& url);

private:
    void saveRefreshToken(const QString& email);
    void loadRefreshToken(const QString& email);
    Device* findDevice(const QString& devId);

    void parseTokenReply(const QJsonDocument& doc);
    void parseDeviceInfoReply(const QJsonDocument& doc);

    QString extractErrorMessage(const QJsonDocument& doc);
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
    QString currentEmail;

    QList<Device> devices;
    QList<QString> devIdsQueue;
    QList<DeviceInfo> devInfoList;
};

} // namespace CUR
