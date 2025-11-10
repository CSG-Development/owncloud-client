#include "remoteconnector.h"
#include "configfile.h"

#include <QRestReply>
#include <QSslCertificate>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QLoggingCategory>

namespace {
auto api_ra_url()         {return QStringLiteral("https://hc-remote-access-env-https.eba-a2nvhpbm.us-west-2.elasticbeanstalk.com/api");}
auto api_ra_initiate()    {return QStringLiteral("/client/v1/auth/initiate");}           // POST
auto api_ra_refresh()     {return QStringLiteral("/client/v1/auth/refresh");}             // GET
auto api_ra_token()       {return QStringLiteral("/client/v1/auth/token");}                 // POST
auto api_ra_devices()     {return QStringLiteral("/client/v1/devices");}                  // GET
auto api_ra_device_info() {return QStringLiteral("/client/v1/devices/");}             // GET

// JSON keys
auto jkey_name()      {return QStringLiteral("name");}
auto jkey_email()     {return QStringLiteral("email");}
auto jkey_code()      {return QStringLiteral("code");}
auto jkey_reference() {return QStringLiteral("reference");}
auto jkey_hostname()  {return QStringLiteral("hostname");}
auto jkey_address()   {return QStringLiteral("address");}
auto jkey_port()      {return QStringLiteral("port");}
auto jkey_seagateDeviceID() {return QStringLiteral("seagateDeviceID");}
//auto jkey_clientFriendlyName() {return QStringLiteral("clientFriendlyName");}
//auto jkey_clientId() {return QStringLiteral("clientId");}
auto jkey_certificateCommonName() {return QStringLiteral("certificateCommonName");}
auto jkey_friendlyName() {return QStringLiteral("friendlyName");}
auto jkey_paths()        {return QStringLiteral("paths");}
auto jkey_type()         {return QStringLiteral("type");}
auto jkey_refreshToken() {return QStringLiteral("refreshToken");}
auto jkey_accessToken()  {return QStringLiteral("accessToken");}
auto jkey_expiresIn()    {return QStringLiteral("expiresIn");}
auto jkey_tokenType()    {return QStringLiteral("tokenType");}
auto jkey_stacktrace()   {return QStringLiteral("stacktrace");}
auto jkey_reason()       {return QStringLiteral("reason");}
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

RemoteConnector::RemoteConnector(QObject *parent)
    : QObject(parent)
    , rest_mgr(std::make_unique<QRestAccessManager>(&net_mgr))
{
    qRegisterMetaType<DeviceInfo>();
    qRegisterMetaType<Device>();

    ::load_loginservices_res();
    rest_factory = std::make_unique<QNetworkRequestFactory>();

    rest_factory->setBaseUrl(QUrl(api_ra_url()));

    connect(&net_mgr, &QNetworkAccessManager::finished, this, [&](QNetworkReply *reply){
        Q_UNUSED(reply);
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
        Q_UNUSED(reply);
        // TODO: custom certificate check
        // call reply->abort() if checks fails
        // qInfo() << "encrypted" << reply->url();
    });

    connect(this, &RemoteConnector::initiate_finished, this, [&](std::optional<QJsonDocument> doc, int code) {
        qCDebug(lcLoginService) << "Initiate request finished, code" << code;
        if (doc) {
            if (code == 200) {
                referenceCode = (*doc)[jkey_reference()].toString();
                emit code_requested();
            }
            else {
                emit error_occured(RemoteRequest::Initiate, code, extractErrorMessage(doc.value()));
            }
        }
        else {
            qCWarning(lcLoginService) << "initiate request returns empty data, code" << code;
            emit error_occured(RemoteRequest::Initiate, code, {});
        }
    });

    connect(this, &RemoteConnector::refresh_finished, this, [&](std::optional<QJsonDocument> doc, int code) {
        qCDebug(lcLoginService) << "Refresh request finished, code" << code;

        if (doc) {
            if (code == 200) {
                parseTokenReply(doc.value());
                query_devices_list(accessToken);
            }
            else {
                emit error_occured(RemoteRequest::Refresh, code, extractErrorMessage(doc.value()));
            }
        }
        else  {
            qCWarning(lcLoginService) << "refresh request returns empty data, code" << code;
            emit error_occured(RemoteRequest::Refresh, code, {});
        }
    });

    connect(this, &RemoteConnector::token_finished, this, [&](std::optional<QJsonDocument> doc, int code) {
        qCDebug(lcLoginService) << "Token request finished, code" << code;

        if (doc) {
            if (code == 200) {
                parseTokenReply(doc.value());
                emit token_success();
                emit query_devices_list(accessToken);
            }
            else {
                auto error_str = doc.value()[jkey_name()].toString();
                emit error_occured(RemoteRequest::Token, code, extractErrorMessage(doc.value()));
            }
        }
        else {
            qCWarning(lcLoginService) << "token request returns empty data, code" << code;
            emit error_occured(RemoteRequest::Token, code, {});
        }
    });

    connect(this, &RemoteConnector::devices_list_finished, this, [&](std::optional<QJsonDocument> doc, int code) {
        qCDebug(lcLoginService) << "Devices list request finished, code" << code;

        if (doc) {
            if (code == 200) {
                devices.clear();
                devInfoList.clear();
                devIdsQueue.clear();

                if (doc->isArray()) {
                    auto arr = doc->array();
                    for (const auto& item : doc->array()) {
                        addDevice(item);
                    }
                }
                emit fetch_devices();
            }
            else {
                auto error_str = doc.value()[jkey_name()].toString();
                emit error_occured(RemoteRequest::DeviceList, code, extractErrorMessage(doc.value()));
            }
        }
        else {
            qCWarning(lcLoginService) << "devices list request returns empty data";
            emit error_occured(RemoteRequest::DeviceList, code, {});
        }
    });

    connect(this, &RemoteConnector::device_info_finished, this, [&](std::optional<QJsonDocument> doc, int code) {
        qCDebug(lcLoginService) << "Device info request finished, code" << code;

        if (doc) {
            if (code == 200) {
                parseDeviceInfoReply(doc.value());
                emit fetch_devices();
            }
            else {
                auto error_str = doc.value()[jkey_name()].toString();
                emit error_occured(RemoteRequest::DeviceInfo, code, extractErrorMessage(doc.value()));
            }
        }
        else {
            qCWarning(lcLoginService) << "device info request returns empty data, code" << code;
            emit error_occured(RemoteRequest::DeviceInfo, code, {});
        }
    });

    connect(this, &RemoteConnector::fetch_devices, this, [&] {
        qCDebug(lcLoginService) << "fetch_devices";
        if (!devIdsQueue.isEmpty()) {
            auto devId = devIdsQueue.takeFirst();
            query_device_info(accessToken, devId);
        }
        else {
            prepareDevList();
            qCDebug(lcLoginService) << "fetch_devices finished";
            emit fetch_devices_finished();
        }
    });
}

RemoteConnector::~RemoteConnector()
{
    ::unload_loginservices_res();
}

void RemoteConnector::setRaCert()
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

void RemoteConnector::start_query(const QString &email)
{
    qCDebug(lcLoginService) << "start_query" << email;
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
            query_devices_list(accessToken);
        }
    }
}

void RemoteConnector::query_initiate(const QString &email)
{
    qCDebug(lcLoginService) << "query_initiate";

    QJsonDocument doc;
    QJsonObject obj;
    obj[QStringLiteral("clientFriendlyName")] = tr("Curator Manager");
    obj[QStringLiteral("clientId")] = tr("");
    obj[jkey_email()] = email;
    doc.setObject(obj);

    rest_factory->setQueryParameters({qMakePair(QStringLiteral("type"), QStringLiteral("email"))});

    auto req = rest_factory->createRequest(api_ra_initiate());
    rest_mgr->post(req, doc, this, [&](QRestReply &reply) {
        emit initiate_finished(reply.readJson(), reply.httpStatus());
    });
}

void RemoteConnector::query_refresh(const QString& refresh_token)
{
    qCDebug(lcLoginService) << "query_refresh";

    rest_factory->setQueryParameters({qMakePair(QStringLiteral("refresh_token"), refresh_token)});

    auto req = rest_factory->createRequest(api_ra_refresh());
    rest_mgr->get(req, this, [&](QRestReply &reply) {
        emit refresh_finished(reply.readJson(), reply.httpStatus());
    });
}

void RemoteConnector::query_token(const QString& code)
{
    qCDebug(lcLoginService) << "query_token";

    QJsonDocument doc;
    QJsonObject payload;
    payload[jkey_code()] = code;
    payload[jkey_reference()] = referenceCode;
    doc.setObject(payload);

    rest_factory->setQueryParameters({qMakePair(QStringLiteral("type"), QStringLiteral("email"))});

    auto req = rest_factory->createRequest(api_ra_token());
    rest_mgr->post(req, doc, this, [&](QRestReply &reply) {
        emit token_finished(reply.readJson(), reply.httpStatus());
    });
}

void RemoteConnector::query_devices_list(const QString& access_token)
{
    qCDebug(lcLoginService) << "query_devices" << access_token;

    rest_factory->clearQueryParameters();

    auto req = rest_factory->createRequest(api_ra_devices());
    req.setRawHeader("authorization", QStringLiteral("Bearer %1").arg(access_token).toUtf8());

    rest_mgr->get(req, this, [&](QRestReply &reply) {
        emit devices_list_finished(reply.readJson(), reply.httpStatus());
    });
}

void RemoteConnector::query_device_info(const QString &access_token, const QString &deviceId)
{
    qCDebug(lcLoginService) << "query_device_info" << deviceId;
    rest_factory->clearQueryParameters();

    auto reqStr = QStringLiteral("%1%2").arg(api_ra_device_info()).arg(deviceId);

    auto req = rest_factory->createRequest(reqStr);
    req.setRawHeader("authorization", QStringLiteral("Bearer %1").arg(access_token).toUtf8());

    rest_mgr->get(req, this, [&](QRestReply &reply) {
        emit device_info_finished(reply.readJson(), reply.httpStatus());
    });
}

QString RemoteConnector::RemoteRequestToStr(RemoteRequest req)
{
    QMap<RemoteRequest,QString> map = {
        {RemoteRequest::Initiate, QStringLiteral("Initiate")},
        {RemoteRequest::Refresh, QStringLiteral("Refresh")},
        {RemoteRequest::Token, QStringLiteral("Token")},
        {RemoteRequest::DeviceList, QStringLiteral("DeviceList")},
        {RemoteRequest::DeviceInfo, QStringLiteral("DeviceInfo")},
    };
    if (map.contains(req))
        return map[req];
    return {};
}

void RemoteConnector::saveRefreshToken(const QString& email)
{
    if (email.isEmpty()) {
        qCWarning(lcLoginService) << "Can't save refresh token with empty email";
        return;
    }
    ConfigFile cf;
    // TODO: use qt6keychain
    QByteArray ba = refreshToken.toLatin1();
    QString enc = QString::fromUtf8(ba.toBase64());
    cf.setRefreshTokenForEmail(enc, email);
}

void RemoteConnector::loadRefreshToken(const QString& email)
{
    if (email.isEmpty()) {
        qCWarning(lcLoginService) << "Can't load refresh token with empty email";
        return;
    }
    ConfigFile cf;
    // TODO: use qt6keychain
    QString dec = cf.refreshTokenForEmail(email);
    QByteArray ba = QByteArray::fromBase64(dec.toUtf8());
    refreshToken = QString::fromUtf8(ba);
}

void RemoteConnector::parseTokenReply(const QJsonDocument& doc)
{
    refreshToken = doc[jkey_refreshToken()].toString();
    accessToken = doc[jkey_accessToken()].toString();
    int expiresIn = doc[jkey_expiresIn()].toInt();
    tokenType = doc[jkey_tokenType()].toString();

    if (expiresIn > 0)
        accessTokenExpireTime = QDateTime::currentDateTime().addSecs(expiresIn);

    saveRefreshToken(currentEmail);
}

void RemoteConnector::parseDeviceInfoReply(const QJsonDocument &doc)
{
    QString devId = doc[jkey_seagateDeviceID()].toString();
    const auto& paths = doc[jkey_paths()].toArray();

    if (auto d = findDevice(devId)) {
        for (const auto p: paths) {
            DevicePath dpath;
            dpath.deviceType = DevicePath::strToDevType(p[jkey_type()].toString());
            dpath.address = p[jkey_address()].toString();
            dpath.port = p[jkey_port()].toInt();
            d->paths.append(dpath);
            qCDebug(lcLoginService) << "Device" << d->seagateDeviceID << "path" << dpath.address;
        }
    }
}

QString RemoteConnector::extractErrorMessage(const QJsonDocument &doc)
{
    QString message;
    const auto reason = doc[jkey_reason()].toString();

    if (reason.isEmpty()) {
        message = reason;
    }
    else {
        message = doc[jkey_stacktrace()].toString();
    }

    if (message.contains(QStringLiteral(":"))) {
        const auto& parts = message.split(QStringLiteral(":"), Qt::SkipEmptyParts);
        if (!parts.isEmpty())
            message = parts.last().trimmed();
    }

    return message;
}

void RemoteConnector::addDevice(const QJsonValue &val)
{
    Device d;
    d.seagateDeviceID = val[jkey_seagateDeviceID()].toString();
    d.certificateCommonName = val[jkey_certificateCommonName()].toString();
    d.friendlyName = val[jkey_friendlyName()].toString();
    d.hostname = val[jkey_hostname()].toString();
    devices.append(d);
    devIdsQueue.append(d.seagateDeviceID);
    qCDebug(lcLoginService) << "Device" << d.seagateDeviceID << d.certificateCommonName << "added";
}

void RemoteConnector::prepareDevList()
{
    devInfoList.clear();
    for (const auto& dev: std::as_const(devices)) {
        for (const auto& path: dev.paths) {
            DeviceInfo di;
            di.name = dev.friendlyName;
            di.host = path.address;
            di.port = path.port;
            di.deviceType = path.deviceType;
            devInfoList.append(di);
        }
    }
}

Device* RemoteConnector::findDevice(const QString &devId)
{
    for (Device& d: devices) {
        if (d.seagateDeviceID == devId)
            return &d;
    }
    return nullptr;
}

} // namespace CUR
