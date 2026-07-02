#include "deviceapi.h"

#include <QNetworkInterface>
#include <QRestReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QLoggingCategory>
#include <QtConcurrent>

namespace {
const QString api_dev_about = QStringLiteral("/api/v1/about"); // GET
const QString api_dev_status = QStringLiteral("/api/v1/status"); // GET
const QString api_users_reset_password = QStringLiteral("/api/v1/users/reset_password/"); // POST
}

Q_LOGGING_CATEGORY(lcDeviceApi, "device.api", QtDebugMsg)

DeviceApi::DeviceApi(QObject *parent)
    : QObject(parent)
    , _rest(new QNetworkAccessManager(this))
{
    connect(_rest.networkAccessManager(), &QNetworkAccessManager::sslErrors, this, [](QNetworkReply *reply, const QList<QSslError> &/*errors*/) {
        reply->ignoreSslErrors();
    });
}

int DeviceApi::aboutTimeoutForDeviceType(DeviceType deviceType)
{
    return deviceType == DeviceType::Local ? LocalAboutTimeoutMs : NonLocalAboutTimeoutMs;
}

QFuture<AboutCtx> DeviceApi::query_about(const QString &url, DeviceType deviceType)
{
    _factory.setBaseUrl(QUrl(url));
    auto request = _factory.createRequest(api_dev_about);
    request.setTransferTimeout(aboutTimeoutForDeviceType(deviceType));
    auto reply = _rest.get(request);
    qCDebug(lcDeviceApi) << "query_about request:" << reply->url();
    return execRequest<AboutCtx>(std::move(reply), [url=reply->url()](const std::optional<QJsonDocument>& doc, int status) {
        AboutCtx ctx;
        ctx.status = status;
        if (doc && !doc->isNull()) {
            qCDebug(lcDeviceApi) << url << "reply:" << doc;
            if (status == 200) {
                ctx.deviceAbout = DeviceInfoAbout::fromJson(doc.value());
            }
            else {
                //ctx.errorString = (*doc)[jkey_name].toString();
                ctx.deviceAbout = {};
                qCWarning(lcDeviceApi) << url << "about status code:" << status;
            }
        }
        else {
            qCDebug(lcDeviceApi) << url << "No reply";
        }
        return ctx;
    });
}

QFuture<StatusCtx> DeviceApi::query_status(const QString &url)
{
    _factory.setBaseUrl(QUrl(url));
    auto request = _factory.createRequest(api_dev_status);
    request.setTransferTimeout(StatusTimeoutMs);
    auto reply = _rest.get(request);
    qCDebug(lcDeviceApi) << "query_status request:" << reply->url();
    return execRequest<StatusCtx>(std::move(reply), [url=reply->url()](const std::optional<QJsonDocument>& doc, int status) {
        StatusCtx ctx;
        ctx.status = status;
        if (doc && !doc->isNull()) {
            qCDebug(lcDeviceApi) << url << "reply:" << doc;
            if (status == 200) {
                ctx.deviceStatus = DeviceInfoStatus::fromJson(doc.value());
            }
            else {
                //ctx.errorString = (*doc)[jkey_name].toString();
                qCWarning(lcDeviceApi) << url << "status code:" << status;
            }
        }
        else {
            qCDebug(lcDeviceApi) << url << "No reply";
        }
        return ctx;
    });
}

QFuture<ResetPasswordCtx> DeviceApi::post_reset_password(const QString &url, const QString &email)
{
    _factory.setBaseUrl(QUrl(url));
    const auto encodedEmail = QString::fromUtf8(QUrl::toPercentEncoding(email.trimmed()));
    auto request = _factory.createRequest(api_users_reset_password + encodedEmail);
    request.setTransferTimeout(StatusTimeoutMs);
    auto reply = _rest.networkAccessManager()->post(request, QByteArray());
    qCDebug(lcDeviceApi) << "post_reset_password request:" << reply->url();
    return execRequest<ResetPasswordCtx>(std::move(reply), [url=reply->url()](const std::optional<QJsonDocument>& doc, int status) {
        ResetPasswordCtx ctx;
        ctx.status = status;

        if (doc && !doc->isNull()) {
            qCDebug(lcDeviceApi) << url << "reply:" << doc;
            const auto obj = doc->object();
            ctx.errorString = obj.value(QStringLiteral("reason")).toString();
            if (ctx.errorString.isEmpty()) {
                ctx.errorString = obj.value(QStringLiteral("name")).toString();
            }
        }
        else if (status != 204) {
            qCDebug(lcDeviceApi) << url << "No reply";
        }

        if (status != 204) {
            qCWarning(lcDeviceApi) << url << "reset password status code:" << status << ctx.errorString;
        }

        return ctx;
    });
}

QFuture<QList<DevicePath> > DeviceApi::query_status_all(QList<DevicePath> paths)
{
    QList<QFuture<StatusCtx>> futures;
    for (const auto& path : paths) {
        QString url = DevHelpers::makeServerUrl(path.address, path.port, false, true, path.origin);
        futures.append(this->query_status(url));
    }

    return QtFuture::whenAll(futures.begin(), futures.end()).then([paths](QList<QFuture<StatusCtx>> finishedFutures) {
        QList<DevicePath> updatedPaths = paths;
        for (int i = 0; i < finishedFutures.size(); ++i) {
            auto ctx = finishedFutures[i].result();
            updatedPaths[i].status = ctx.deviceStatus;
            updatedPaths[i].statusHttpStatus = ctx.status;
            if (ctx.status != 200) {
                qCWarning(lcDeviceApi) << "status API returned error status:" << ctx.status;
            }
        }
        return updatedPaths;
    });
}

QFuture<std::pair<AboutCtx, StatusCtx> > DeviceApi::query_about_status(const QString &url, DeviceType deviceType)
{
    auto futureAbout = query_about(url, deviceType);
    auto futureStatus = query_status(url);

    return QtFuture::whenAll(futureAbout, futureStatus).then([futureAbout, futureStatus](auto &&) {
        return std::make_pair(futureAbout.result(), futureStatus.result());
    });
}


QFuture<QList<DevicePath>> DeviceApi::query_about_all(QList<DevicePath> paths)
{
    QList<QFuture<AboutCtx>> futures;
    for (const auto& path : paths) {
        QString url = DevHelpers::makeServerUrl(path.address, path.port, false, true, path.origin);
        futures.append(this->query_about(url, path.deviceType));
    }

    return QtFuture::whenAll(futures.begin(), futures.end()).then([paths](QList<QFuture<AboutCtx>> finishedFutures) {
        QList<DevicePath> updatedPaths = paths;
        for (int i = 0; i < finishedFutures.size(); ++i) {
            auto ctx = finishedFutures[i].result();
            updatedPaths[i].aboutHttpStatus = ctx.status;
            if (ctx.status == 200) {
                updatedPaths[i].about = ctx.deviceAbout;
            }
            else {
                qCWarning(lcDeviceApi) << "API returned error status:" << ctx.status;
            }
        }
        return updatedPaths;
    });
}
