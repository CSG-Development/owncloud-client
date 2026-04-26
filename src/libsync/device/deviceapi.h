#pragma once

#include "personalcloudlib.h"
#include "devicetypes.h"

#include <QObject>
#include <QNetworkAccessManager>
#include <QRestAccessManager>
#include <QNetworkRequestFactory>
#include <QNetworkReply>
#include <QRestReply>
#include <QJsonDocument>
#include <QFuture>
#include <QPromise>

struct AboutCtx {
    DeviceInfoAbout deviceAbout;
    QString errorString;
    int status = 0;
};
struct StatusCtx {
    DeviceInfoStatus deviceStatus;
    QString errorString;
    int status = 0;
};
struct ResetPasswordCtx {
    QString errorString;
    int status = 0;
};

class APPLICATIONSYNC_EXPORT DeviceApi: public QObject
{
    Q_OBJECT

public:
    explicit DeviceApi(QObject* parent = nullptr);

    QFuture<AboutCtx> query_about(const QString &url, DeviceType deviceType = DeviceType::Unknown);
    QFuture<StatusCtx> query_status(const QString &url);
    QFuture<ResetPasswordCtx> post_reset_password(const QString &url, const QString &email);

    QFuture<QList<DevicePath>> query_about_all(QList<DevicePath> paths);
    QFuture<QList<DevicePath>> query_status_all(QList<DevicePath> paths);

    QFuture<std::pair<AboutCtx,StatusCtx>> query_about_status(const QString &url, DeviceType deviceType = DeviceType::Unknown);

protected:
    static int aboutTimeoutForDeviceType(DeviceType deviceType);
    static constexpr int LocalAboutTimeoutMs = 4 * 1000;
    static constexpr int NonLocalAboutTimeoutMs = 9 * 1000;
    static constexpr int StatusTimeoutMs = 5 * 1000;

    template <typename T>
    QFuture<T> execRequest(QNetworkReply* netReply, std::function<T(const std::optional<QJsonDocument>&,int)> parser) {
        auto promise = std::make_shared<QPromise<T>>();
        auto future = promise->future();

        if (!netReply) {
            promise->finish();
            return future;
        }

        connect(netReply, &QNetworkReply::finished, this, [promise, parser, netReply]() mutable {

            QRestReply restReply(netReply);
            int statusCode = restReply.httpStatus();
            auto doc = restReply.readJson();

            try {
                promise->addResult(parser(doc, statusCode));
            } catch (const std::exception &) {
                //promise->setException(std::make_exception_ptr(e));
                T errCtx;
                errCtx.status = -1;
                promise->addResult(errCtx);
            }
            promise->finish();
            netReply->deleteLater();
        });

        return future;
    }

    QRestAccessManager _rest;
    QNetworkRequestFactory _factory;
};
