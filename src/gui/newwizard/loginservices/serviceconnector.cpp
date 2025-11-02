#include "serviceconnector.h"
#include "configfile.h"

#include <QRestReply>
#include <QSslCertificate>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QLoggingCategory>

namespace {
const QString ra_cert_file = QStringLiteral("D:/projects/noveo/main/h836/test/src/fake-device-noveo.cer");

const QString api_ra_url = QStringLiteral("https://hc-remote-access-env-https.eba-a2nvhpbm.us-west-2.elasticbeanstalk.com/api");
const QString api_ra_initiate = QStringLiteral("/client/v1/auth/initiate");           // POST
const QString api_ra_refresh = QStringLiteral("/client/v1/auth/refresh");             // GET
const QString api_ra_token = QStringLiteral("/client/v1/auth/token");                 // POST
const QString api_ra_devices = QStringLiteral("/client/v1/devices");                  // GET
const QString api_ra_device_info = QStringLiteral("/client/v1/devices/");             // GET
}

Q_LOGGING_CATEGORY(lcLoginService, "login.service", QtDebugMsg)

void static load_loginservices_res()
{
    Q_INIT_RESOURCE(loginservices_res);
}

void static unload_loginservices_res()
{
    Q_CLEANUP_RESOURCE(loginservices_res);
}

namespace CUR {

ServiceConnector::ServiceConnector(QObject *parent)
    : QObject(parent)
    , rest_mgr(std::make_unique<QRestAccessManager>(&net_mgr))
{
    qRegisterMetaType<DeviceInfo>();

    ::load_loginservices_res();
    rest_factory = std::make_unique<QNetworkRequestFactory>();

    rest_factory->setBaseUrl(QUrl(api_ra_url));

    connect(&net_mgr, &QNetworkAccessManager::finished, this, [&](QNetworkReply *reply){
        //qCDebug(lcLoginService) << "QNetworkAccessManager::finished" << reply->error();
    });
    connect(&net_mgr, &QNetworkAccessManager::sslErrors, this, [&](QNetworkReply *reply, const QList<QSslError> &errors) {
        qCInfo(lcLoginService) << "***";
        qCInfo(lcLoginService) << "URL:" << reply->request().url();
        qCWarning(lcLoginService) << "sslErrors:";
        for (const auto& err: errors)
            qCInfo(lcLoginService) << "  " << err;

        qCInfo(lcLoginService) << "***";
        reply->ignoreSslErrors();
    });
    connect(&net_mgr, &QNetworkAccessManager::encrypted, this, [&](QNetworkReply *reply){
        // TODO: custom certificate check
        // call reply->abort() if checks fails
        // qInfo() << "encrypted" << reply->url();
    });

    connect(this, &ServiceConnector::initiate_finished, this, [&](std::optional<QJsonDocument> doc, int code) {
        if (!doc) {
            qCWarning(lcLoginService) << "initiate returns empty data, code" << code;
            return;
        }

        if (code == 200) {
            referenceCode = (*doc)[QStringLiteral("reference")].toString();
            emit code_requested();
        }
        else {
            emit error_code(code, QStringLiteral("initiate"));
        }
    });
    connect(this, &ServiceConnector::refresh_finished, this, [&](std::optional<QJsonDocument> doc, int code) {
        if (!doc) {
            qCWarning(lcLoginService) << "refresh returns empty data, code" << code;
            return;
        }

        if (code == 200) {
            parseTokenReply(doc.value());
            emit query_devices(accessToken);
        }
        else {
            emit error_code(code, QStringLiteral("refresh"));
        }
    });
    connect(this, &ServiceConnector::token_finished, this, [&](std::optional<QJsonDocument> doc, int code) {
        if (!doc) {
            qCWarning(lcLoginService) << "token returns empty data, code" << code;
            return;
        }

        if (code == 200) {
            parseTokenReply(doc.value());
            emit query_devices(accessToken);
        }
        else {
            emit error_code(code, QStringLiteral("token"));
        }
    });
    connect(this, &ServiceConnector::devices_finished, this, [&](std::optional<QJsonDocument> doc, int code) {
        if (!doc) {
            qCWarning(lcLoginService) << "devices returns empty data, code" << code;
            return;
        }
        if (code == 200) {
            devices.clear();
            devInfoList.clear();
            devIdsQueue.clear();

            if (doc->isArray()) {
                auto arr = doc->array();
                for (const auto& item : doc->array()) {
                    addDevice(item);
                }

                emit fetch_devices();
            }
        }
        else {
            emit error_code(code, QStringLiteral("devices"));
        }
    });

    connect(this, &ServiceConnector::device_info_finished, this, [&](std::optional<QJsonDocument> doc, int code) {
        if (!doc) {
            qCWarning(lcLoginService) << "device info returns empty data, code" << code;
            return;
        }
        if (code == 200) {
            parseDeviceInfoReply(doc.value());
            emit fetch_devices();
        }
        else {
            emit error_code(code, QStringLiteral("device_info"));
        }
    });

    connect(this, &ServiceConnector::fetch_devices, this, [&] {
        if (!devIdsQueue.isEmpty()) {
            auto devId = devIdsQueue.takeFirst();
            query_device_info(accessToken, devId);
        }
        else {
            prepareDevList();
            emit fetch_devices_finished();
        }
    });
}

ServiceConnector::~ServiceConnector()
{
    ::unload_loginservices_res();
}

void ServiceConnector::setRaCert()
{
    QFile f(QStringLiteral(":/res/fake-device-noveo.cer"));
    QByteArray data;
    if (!f.open(QIODevice::ReadOnly)) {
        qCWarning(lcLoginService) << "Can't open certificate resource";
        return;
    }

    data = f.readAll();

    if (data.isEmpty()) {
        qCWarning(lcLoginService) << "No data in the certificate resource";
        return;
    }

    auto certs = QSslCertificate::fromData(data);
    if (certs.isEmpty()) {
        qCWarning(lcLoginService) << "No certs found";
        return;
    }

    ssl_config.setCaCertificates(certs);
    rest_factory->setSslConfiguration(ssl_config);
}

void ServiceConnector::start_query(const QString &email)
{
    currentEmail  = email;
    loadRefreshToken(currentEmail);
    if (refreshToken.isEmpty()) {
        query_initiate(email);
    }
    else {
        if (QDateTime::currentDateTime() > accessTokenExpireTime) {
            query_refresh(refreshToken);
        }
        else {
            query_devices(accessToken);
        }
    }
}

void ServiceConnector::query_initiate(const QString &email)
{
    qCDebug(lcLoginService) << "query_initiate";

    QJsonDocument doc;
    QJsonObject obj;
    obj[QStringLiteral("clientFriendlyName")] = tr("Curator Manager");
    obj[QStringLiteral("clientId")] = tr("");
    obj[QStringLiteral("email")] = email;
    doc.setObject(obj);

    rest_factory->setQueryParameters({qMakePair(QStringLiteral("type"), QStringLiteral("email"))});

    auto req = rest_factory->createRequest(api_ra_initiate);
    rest_mgr->post(req, doc, this, [&](QRestReply &reply) {
        emit initiate_finished(reply.readJson(), reply.httpStatus());
    });
}

void ServiceConnector::query_refresh(const QString& refresh_token)
{
    qCDebug(lcLoginService) << "query_refresh";

    rest_factory->setQueryParameters({qMakePair(QStringLiteral("refresh_token"), refresh_token)});

    auto req = rest_factory->createRequest(api_ra_refresh);
    rest_mgr->get(req, this, [&](QRestReply &reply) {
        emit refresh_finished(reply.readJson(), reply.httpStatus());
    });
}

void ServiceConnector::query_token(const QString& code)
{
    qCDebug(lcLoginService) << "query_token";

    QJsonDocument doc;
    QJsonObject payload;
    payload[QStringLiteral("code")] = code;
    payload[QStringLiteral("reference")] = referenceCode;
    doc.setObject(payload);

    rest_factory->setQueryParameters({qMakePair(QStringLiteral("type"), QStringLiteral("email"))});

    auto req = rest_factory->createRequest(api_ra_token);
    rest_mgr->post(req, doc, this, [&](QRestReply &reply) {
        emit token_finished(reply.readJson(), reply.httpStatus());
    });
}

void ServiceConnector::query_devices(const QString& access_token)
{
    qCDebug(lcLoginService) << "query_devices" << access_token;

    rest_factory->clearQueryParameters();

    auto req = rest_factory->createRequest(api_ra_devices);
    req.setRawHeader("authorization", QStringLiteral("Bearer %1").arg(access_token).toUtf8());

    rest_mgr->get(req, this, [&](QRestReply &reply) {
        emit devices_finished(reply.readJson(), reply.httpStatus());
    });
}

void ServiceConnector::query_device_info(const QString &access_token, const QString &deviceId)
{
    qCDebug(lcLoginService) << "query_device_info" << deviceId;
    rest_factory->clearQueryParameters();

    auto reqStr = QStringLiteral("%1%2").arg(api_ra_device_info).arg(deviceId);

    auto req = rest_factory->createRequest(reqStr);
    req.setRawHeader("authorization", QStringLiteral("Bearer %1").arg(access_token).toUtf8());

    rest_mgr->get(req, this, [&](QRestReply &reply) {
        emit device_info_finished(reply.readJson(), reply.httpStatus());
    });
}

void ServiceConnector::saveRefreshToken(const QString& email)
{
    if (email.isEmpty()) {
        qCWarning(lcLoginService) << "Can't save refresh token with empty email";
        return;
    }
    ConfigFile cf;
    cf.setRefreshTokenForEmail(refreshToken, email);
}

void ServiceConnector::loadRefreshToken(const QString& email)
{
    if (email.isEmpty()) {
        qCWarning(lcLoginService) << "Can't load refresh token with empty email";
        return;
    }
    ConfigFile cf;
    refreshToken = cf.refreshTokenForEmail(email);
}

void ServiceConnector::parseTokenReply(const QJsonDocument& doc)
{
    refreshToken = doc[QStringLiteral("refreshToken")].toString();
    accessToken = doc[QStringLiteral("accessToken")].toString();
    int expiresIn = doc[QStringLiteral("expiresIn")].toInt();
    tokenType = doc[QStringLiteral("tokenType")].toString();

    if (expiresIn > 0)
        accessTokenExpireTime = QDateTime::currentDateTime().addSecs(expiresIn);

    saveRefreshToken(currentEmail);
}

void ServiceConnector::parseDeviceInfoReply(const QJsonDocument &doc)
{
    QString devId = doc[QStringLiteral("seagateDeviceID")].toString();
    const auto& paths = doc[QStringLiteral("paths")].toArray();

    if (auto d = findDevice(devId)) {
        for (const auto p: paths) {
            DevicePath dpath;
            dpath.deviceType = DevicePath::strToDevType(p[QStringLiteral("type")].toString());
            dpath.address = p[QStringLiteral("address")].toString();
            dpath.port = p[QStringLiteral("port")].toInt();
            d->paths.append(dpath);
            qCDebug(lcLoginService) << "Device" << d->seagateDeviceID << "path" << dpath.address;
        }
    }
}

void ServiceConnector::addDevice(const QJsonValue &val)
{
    Device d;
    d.seagateDeviceID = val[QStringLiteral("seagateDeviceID")].toString();
    d.certificateCommonName = val[QStringLiteral("certificateCommonName")].toString();
    d.friendlyName = val[QStringLiteral("friendlyName")].toString();
    d.hostname = val[QStringLiteral("hostname")].toString();
    devices.append(d);
    devIdsQueue.append(d.seagateDeviceID);
    qCDebug(lcLoginService) << "Device" << d.seagateDeviceID << "added";
}

void ServiceConnector::prepareDevList()
{
    devInfoList.clear();
    for (const auto& dev: devices) {
        for (const auto& path: dev.paths) {
            DeviceInfo di;
            di.name = dev.friendlyName;
            di.host = path.address;
            di.port = path.port;
            devInfoList.append(di);
        }
    }
}

Device* ServiceConnector::findDevice(const QString &devId)
{
    for (Device& d: devices) {
        if (d.seagateDeviceID == devId)
            return &d;
    }
    return nullptr;
}

} // namespace CUR
