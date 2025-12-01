#include "deviceapi.h"

#include <QNetworkInterface>
#include <QRestReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QLoggingCategory>

namespace {
const QString api_dev_about = QStringLiteral("/api/v1/about"); // GET
const QString api_dev_status = QStringLiteral("/api/v1/status"); // GET
}

Q_LOGGING_CATEGORY(lcDeviceApi, "device.api", QtDebugMsg)

namespace CUR {

DeviceApi::DeviceApi(QObject *parent)
    : QObject(parent)
    , rest_mgr(std::make_unique<QRestAccessManager>(&net_mgr))
{
    net_mgr.setTransferTimeout(5 * 1000);
    rest_factory = std::make_unique<QNetworkRequestFactory>();

    connect(&net_mgr, &QNetworkAccessManager::sslErrors, this, [&](QNetworkReply *reply, const QList<QSslError>&/*errors*/) {
        reply->ignoreSslErrors();
    });

    connect(this, &DeviceApi::about_request_finished, this, [&](std::optional<QJsonDocument> doc, int code) {
        DeviceInfoAbout localDevice;

        if (code == 200) {
            if (doc) {
                localDevice = DeviceInfoAbout::fromJson(doc.value());
            }
            else {
                qCWarning(lcDeviceApi) << "device about returns empty data, code" << code;
            }
        }
        else {
            if (doc) {
                auto error_str = doc.value()[QStringLiteral("message")].toString();
                qCWarning(lcDeviceApi) << code << error_str;
            }
            else {
                qCWarning(lcDeviceApi) << "about_request error" << code;
            }
        }

        emit about_finished(localDevice, code);
    });

    connect(this, &DeviceApi::status_request_finished, this, [&](std::optional<QJsonDocument> doc, int code) {
        DeviceInfoStatus devStatus;

        if (!doc) {
            qCWarning(lcDeviceApi) << "device status returns empty data, code" << code;
        }
        if (code == 200) {
            if (doc) {
                devStatus = DeviceInfoStatus::fromJson(doc.value());
            }
            else {
                qCWarning(lcDeviceApi) << "device status returns empty data, code" << code;
            }
        }
        else {
            if (doc) {
                auto error_str = doc.value()[QStringLiteral("message")].toString();
                qCWarning(lcDeviceApi) << code << error_str;
            }
        }

        emit status_finished(devStatus, code);
    });
}

DeviceApi::~DeviceApi()
{
}

void DeviceApi::query_device_about(const QString &url)
{
    rest_factory->setBaseUrl(QUrl(url));

    auto req = rest_factory->createRequest(api_dev_about);
    rest_mgr->get(req, this, [&](QRestReply &reply) {
        emit about_request_finished(reply.readJson(), reply.httpStatus());
    });
}

void DeviceApi::query_device_status(const QString& url)
{
    rest_factory->setBaseUrl(QUrl(url));

    auto req = rest_factory->createRequest(api_dev_status);
    rest_mgr->get(req, this, [&](QRestReply &reply) {
        emit status_request_finished(reply.readJson(), reply.httpStatus());
    });
}

} // namespace CUR
