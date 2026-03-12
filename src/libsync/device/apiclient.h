#pragma once

#include "personalcloudlib.h"
#include "device/devicetypes.h"

#include <QRestAccessManager>
#include <QNetworkRequestFactory>
#include <QFuture>
#include <QPromise>
#include <QNetworkReply>
#include <QRestReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

struct ResultContext {
    QString errorString;
    QString errorStacktrace;
    int status = 0;
};

struct InitContext {
    QString refCode;
    ResultContext res;
};

struct TokenContext {
    QString accessToken;
    QString refreshToken;
    QString tokenType;
    QDateTime accessTokenExpireTime;
    int expiresIn = 0;
    ResultContext res;
};

struct DeviceListCtx {
    DeviceList deviceList;
    ResultContext res;
};

struct DevicePathListCtx {
    QList<DevicePath> devicePathList;
    ResultContext res;
};

class APPLICATIONSYNC_EXPORT ApiClient : public QObject
{
    Q_OBJECT

public:
    explicit ApiClient(QObject *parent = nullptr);

    QFuture<InitContext> ra_initiate(const QString& email);
    QFuture<TokenContext> ra_token(const QString& code);
    QFuture<DeviceListCtx> ra_device_list();
    QFuture<DevicePathListCtx> ra_device_info(const QString& deviceId);
    QFuture<TokenContext> ra_refresh();

    void setClientId(const QString& clientId) {_clientId = clientId;}
    void setRefreshToken(const QString& refreshToken);
    bool hasRefreshToken() const;

    const TokenContext& tokenCtx() const {return _tokenCtx;}
    void setTokenCtx(TokenContext&& ctx);
    void clearTokens();


protected:

    template <typename T>
    QFuture<T> execRequest(QNetworkReply* netReply, std::function<T(const std::optional<QJsonDocument>&,int)> parser) {
        auto promise = std::make_shared<QPromise<T>>();
        auto future = promise->future();

        if (!netReply) {
            promise->addResult(T());
            promise->finish();
            return future;
        }

        connect(netReply, &QNetworkReply::finished, this, [promise, parser, netReply]() mutable {

            // defer netReply->deleteLater();
            auto deleter = [](QObject* obj) { if (obj) obj->deleteLater(); };
            std::unique_ptr<QNetworkReply, decltype(deleter)> guard(netReply, deleter);

            QRestReply restReply(netReply);
            int statusCode = restReply.httpStatus();

            auto doc = restReply.readJson();

            if (statusCode != 200)
                qWarning() << "execRequest code" << statusCode << doc;

            try {
                promise->addResult(parser(doc, statusCode));
            } catch (const std::exception& e) {
                T errCtx;
                errCtx.res.status = -1;
                promise->addResult(errCtx);
            }

            promise->finish();
        });

        return future;
    }

    QFuture<TokenContext> ensureAuthenticated();
    bool isAccessTokenValid() const;

    TokenContext parseTokenContext(const std::optional<QJsonDocument>& doc, int status);

    QFuture<TokenContext> _authFuture;
    std::shared_ptr<QPromise<void>> _pendingAuthPromise;

    QRestAccessManager _rest;
    QNetworkRequestFactory _factory;
    TokenContext _tokenCtx;
    InitContext _initContext;
    QString _clientId;
    QTimer _codeTimer;
};
