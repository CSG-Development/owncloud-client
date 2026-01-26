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
}

Q_LOGGING_CATEGORY(lcDeviceApi, "device.api", QtDebugMsg)

DeviceApi::DeviceApi(QObject *parent)
    : QObject(parent)
    , _rest(new QNetworkAccessManager(this))
{
    _rest.networkAccessManager()->setTransferTimeout(5 * 1000);

    connect(_rest.networkAccessManager(), &QNetworkAccessManager::sslErrors, this, [](QNetworkReply *reply, const QList<QSslError> &/*errors*/) {
        reply->ignoreSslErrors();
    });
}

QFuture<AboutCtx> DeviceApi::query_about(const QString &url)
{
    _factory.setBaseUrl(QUrl(url));
    auto reply = _rest.get(_factory.createRequest(api_dev_about));
    return execRequest<AboutCtx>(std::move(reply), [](const std::optional<QJsonDocument>& doc, int status) {
        AboutCtx ctx;
        ctx.status = status;
        if (doc && !doc->isNull()) {
            if (status == 200) {
                ctx.deviceAbout = DeviceInfoAbout::fromJson(doc.value());
            }
            else {
                //ctx.errorString = (*doc)[jkey_name].toString();
                ctx.deviceAbout = {};
                qCWarning(lcDeviceApi) << "about status code:" << status;
            }
        }
        return ctx;
    });
}

QFuture<StatusCtx> DeviceApi::query_status(const QString &url)
{
    qCDebug(lcDeviceApi) << "query_status";
    _factory.setBaseUrl(QUrl(url));
    auto reply = _rest.get(_factory.createRequest(api_dev_status));
    return execRequest<StatusCtx>(std::move(reply), [](const std::optional<QJsonDocument>& doc, int status) {
        qCDebug(lcDeviceApi) << "query_status code" << status;
        StatusCtx ctx;
        ctx.status = status;
        if (doc && !doc->isNull()) {
            if (status == 200) {
                ctx.deviceStatus = DeviceInfoStatus::fromJson(doc.value());
            }
            else {
                //ctx.errorString = (*doc)[jkey_name].toString();
                qCWarning(lcDeviceApi) << "status code:" << status;
            }
        }
        return ctx;
    });
}

QFuture<QList<DevicePath> > DeviceApi::query_status_all(QList<DevicePath> paths)
{
    QList<QFuture<StatusCtx>> futures;
    for (const auto& path : paths) {
        QString url = DevHelpers::makeServerUrl(path.address, path.port, false, true);
        futures.append(this->query_status(url));
    }

    return QtFuture::whenAll(futures.begin(), futures.end()).then([paths](QList<QFuture<StatusCtx>> finishedFutures) {
        QList<DevicePath> updatedPaths = paths;
        for (int i = 0; i < finishedFutures.size(); ++i) {
            auto ctx = finishedFutures[i].result();
            updatedPaths[i].status = ctx.deviceStatus;
            if (ctx.status != 200) {
                qWarning() << "status API returned error status:" << ctx.status;
            }
        }
        return updatedPaths;
    });
}

QFuture<std::pair<AboutCtx, StatusCtx> > DeviceApi::query_about_status(const QString &url)
{
    auto futureAbout = query_about(url);
    auto futureStatus = query_status(url);

    return QtFuture::whenAll(futureAbout, futureStatus).then([futureAbout, futureStatus](auto &&) {
        return std::make_pair(futureAbout.result(), futureStatus.result());
    });
}


QFuture<QList<DevicePath>> DeviceApi::query_about_all(QList<DevicePath> paths)
{
    QList<QFuture<AboutCtx>> futures;
    for (const auto& path : paths) {
        QString url = DevHelpers::makeServerUrl(path.address, path.port, false, true);
        futures.append(this->query_about(url));
    }

    return QtFuture::whenAll(futures.begin(), futures.end()).then([paths](QList<QFuture<AboutCtx>> finishedFutures) {
        QList<DevicePath> updatedPaths = paths;
        for (int i = 0; i < finishedFutures.size(); ++i) {
            auto ctx = finishedFutures[i].result();
            if (ctx.status == 200) {
                updatedPaths[i].about = ctx.deviceAbout;
            }
            else {
                qWarning() << "API returned error status:" << ctx.status;
            }
        }
        return updatedPaths;
    });
}
