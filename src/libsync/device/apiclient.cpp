#include "apiclient.h"
#include <QJsonArray>
#include <QtConcurrent>
#include <QLoggingCategory>

namespace {
const auto api_ra_url         = QStringLiteral("https://hc-remote-access-env-https.eba-a2nvhpbm.us-west-2.elasticbeanstalk.com/api");
const auto api_ra_initiate    = QStringLiteral("/client/v1/auth/initiate");   // POST
const auto api_ra_refresh     = QStringLiteral("/client/v1/auth/refresh");    // GET
const auto api_ra_token       = QStringLiteral("/client/v1/auth/token");      // POST
const auto api_ra_devices     = QStringLiteral("/client/v1/devices");         // GET
const auto api_ra_device_info = QStringLiteral("/client/v1/devices/");        // GET

// JSON keys
const auto jkey_name      = QStringLiteral("name");
const auto jkey_email     = QStringLiteral("email");
const auto jkey_code      = QStringLiteral("code");
const auto jkey_reference = QStringLiteral("reference");
const auto jkey_hostname  = QStringLiteral("hostname");
const auto jkey_address   = QStringLiteral("address");
const auto jkey_port      = QStringLiteral("port");
const auto jkey_seagateDeviceID    = QStringLiteral("seagateDeviceID");
const auto jkey_clientFriendlyName = QStringLiteral("clientFriendlyName");
const auto jkey_clientId     = QStringLiteral("clientId");
const auto jkey_certificateCommonName = QStringLiteral("certificateCommonName");
const auto jkey_friendlyName = QStringLiteral("friendlyName");
const auto jkey_paths        = QStringLiteral("paths");
const auto jkey_type         = QStringLiteral("type");
const auto jkey_refreshToken = QStringLiteral("refreshToken");
const auto jkey_accessToken  = QStringLiteral("accessToken");
const auto jkey_expiresIn    = QStringLiteral("expiresIn");
const auto jkey_tokenType    = QStringLiteral("tokenType");
const auto jkey_stacktrace   = QStringLiteral("stacktrace");
const auto jkey_reason       = QStringLiteral("reason");

DeviceListCtx makeDeviceListError(const TokenContext& tokenCtx)
{
    DeviceListCtx errCtx;
    errCtx.res.status = tokenCtx.res.status;
    errCtx.res.errorString = tokenCtx.res.errorString;
    errCtx.res.errorStacktrace = tokenCtx.res.errorStacktrace;
    return errCtx;
}

DevicePathListCtx makeDevicePathListError(const TokenContext& tokenCtx)
{
    DevicePathListCtx errCtx;
    errCtx.res.status = tokenCtx.res.status;
    errCtx.res.errorString = tokenCtx.res.errorString;
    errCtx.res.errorStacktrace = tokenCtx.res.errorStacktrace;
    return errCtx;
}
}

Q_LOGGING_CATEGORY(lcDeviceApiClient, "device.apiclient", QtDebugMsg)

ApiClient::ApiClient(QObject *parent)
    : QObject(parent)
    , _rest(new QNetworkAccessManager(this))
{
    _factory.setBaseUrl(QUrl(api_ra_url));

    _codeTimer.setSingleShot(true);
    _codeTimer.setInterval(10 * 60 * 1000); // 10 minutes

    connect(_rest.networkAccessManager(), &QNetworkAccessManager::sslErrors, this, [](QNetworkReply *reply, const QList<QSslError> &/*errors*/) {
        reply->ignoreSslErrors();
    });
}

QFuture<InitContext> ApiClient::ra_initiate(const QString &email)
{
    QJsonObject payload;
    payload[jkey_email] = email;
    payload[jkey_clientId] = _clientId;
    payload[jkey_clientFriendlyName] = QStringLiteral("Desktop client");

    _factory.setQueryParameters({qMakePair(QStringLiteral("type"), QStringLiteral("email"))});
    auto reply = _rest.post(_factory.createRequest(api_ra_initiate), QJsonDocument(payload));
    return execRequest<InitContext>(std::move(reply), [this](const std::optional<QJsonDocument>& doc, int status) {
        InitContext ctx;
        ctx.res.status = status;
        if (doc && !doc->isNull()) {
            if (status == 200) {
                _initContext.refCode = (*doc)[jkey_reference].toString();
            }
            else {
                ctx.res.errorString = (*doc)[jkey_name].toString();
                ctx.res.errorStacktrace = (*doc)[jkey_stacktrace].toString();
            }
        }
        return ctx;
    });
}

/**
 * @brief ApiClient::ra_token
 * 400 Bad request parameters
 * 401 Invalid credentials or verification code expired
 * 500 The request failed and returned an error
 */
QFuture<TokenContext> ApiClient::ra_token(const QString &code)
{
    QJsonObject payload;
    payload[jkey_code] = code;
    payload[jkey_reference] = _initContext.refCode;
    payload[jkey_clientId] = _clientId;

    _factory.setQueryParameters({qMakePair(QStringLiteral("type"), QStringLiteral("email"))});
    auto reply = _rest.post(_factory.createRequest(api_ra_token), QJsonDocument(payload));

    return execRequest<TokenContext>(std::move(reply), [this](const std::optional<QJsonDocument>& doc, int status) {
        return parseTokenContext(doc, status);
    });
}

/**
 * @brief ApiClient::ra_device_list
 * 401 Valid token but not allowed to perform this action
 * 403 Invalid authentication
 * 500 The request failed and returned an error
 */
QFuture<DeviceListCtx> ApiClient::ra_device_list()
{
    qCDebug(lcDeviceApiClient) << "[ra_device_list]";

    return ensureAuthenticated().then(this, [this](const TokenContext& authCtx) {
        qCDebug(lcDeviceApiClient) << "[ra_device_list] ensureAuthenticated().then, status" << authCtx.res.status;
        if (authCtx.res.status != 200) {
            return QtFuture::makeReadyValueFuture(makeDeviceListError(authCtx));
        }

        return executeDeviceListRequest(true);
    }).unwrap();
}

/**
 * @brief ApiClient::ra_device_info
 * 401 Valid token but not allowed to perform this action
 * 403 Invalid authentication
 * 404 device not found
 * 500 The request failed and returned an error
 */
QFuture<DevicePathListCtx> ApiClient::ra_device_info(const QString& deviceId)
{
    qCDebug(lcDeviceApiClient) << "[ra_device_info]";

    return ensureAuthenticated().then(this, [this,deviceId](const TokenContext& authCtx) {
        qCDebug(lcDeviceApiClient) << "[ra_device_info] ensureAuthenticated().then, status" << authCtx.res.status;
        if (authCtx.res.status != 200) {
            return QtFuture::makeReadyValueFuture(makeDevicePathListError(authCtx));
        }

        return executeDeviceInfoRequest(deviceId, true);
    }).unwrap();
}

/**
 * @brief ApiClient::ra_refresh
 * 400 invalid request parameters
 * 401 Invalid refresh token or client ID
 * 403 Invalid authentication
 * 500 The request failed and returned an error
 *
 * 429 Too Many Requests (Too Many Requests: retry verification code request after 32 seconds)
 */
QFuture<TokenContext> ApiClient::ra_refresh()
{
    QJsonObject payload;
    payload[jkey_refreshToken] = _tokenCtx.refreshToken;
    payload[jkey_clientId] = _clientId;

    //_factory.clearQueryParameters();
    _factory.setQueryParameters({qMakePair(QStringLiteral("refresh_token"), _tokenCtx.refreshToken)});
    auto reply = _rest.post(_factory.createRequest(api_ra_refresh), QJsonDocument(payload));
    return execRequest<TokenContext>(std::move(reply), [this](const std::optional<QJsonDocument>& doc, int status) {
        return parseTokenContext(doc, status);
    });
}

void ApiClient::setRefreshToken(const QString &refreshToken)
{
    _tokenCtx.refreshToken = refreshToken;
}

void ApiClient::setTokenCtx(TokenContext &&ctx)
{
    _tokenCtx = std::move(ctx);
}

void ApiClient::clearTokens()
{
    _tokenCtx = {};
}

bool ApiClient::hasRefreshToken() const
{
    return !_tokenCtx.refreshToken.isEmpty();
}

QFuture<TokenContext> ApiClient::ensureAuthenticated()
{
    qCDebug(lcDeviceApiClient) << "[ensureAuth]";
    return refreshAccessToken(false);
}

QFuture<TokenContext> ApiClient::refreshAccessToken(bool forceRefresh)
{
    qCDebug(lcDeviceApiClient) << "[refreshAccessToken] force:" << forceRefresh;

    if (!forceRefresh && isAccessTokenValid())
        return QtFuture::makeReadyValueFuture(_tokenCtx);

    if (_authFuture.isRunning())
        return _authFuture;

    if (_tokenCtx.refreshToken.isEmpty()) {
        qCWarning(lcDeviceApiClient) << "[refreshAccessToken] No refresh token available";
        TokenContext errCtx;
        errCtx.res.status = -2;
        errCtx.res.errorString = QStringLiteral("No refresh token available");
        return QtFuture::makeReadyValueFuture(errCtx);
    }

    auto promise = std::make_shared<QPromise<TokenContext>>();
    _authFuture = promise->future();

    qCDebug(lcDeviceApiClient) << "[refreshAccessToken] Starting refresh token";
    ra_refresh()
        .then(this, [this, promise](const TokenContext& ctx) {
            qCDebug(lcDeviceApiClient) << "[refreshAccessToken] Refresh finished" << ctx.res.status;
            _tokenCtx = ctx;
            promise->addResult(_tokenCtx);
            promise->finish();
        }).onFailed(this, [promise](const std::exception &e) {
            qCWarning(lcDeviceApiClient) << "[refreshAccessToken] Transport exception:" << e.what();
            TokenContext errCtx;
            errCtx.res.status = -2;
            errCtx.res.errorString = QString::fromUtf8(e.what());
            promise->addResult(errCtx);
            promise->finish();
        });

    return _authFuture;
}

QFuture<DeviceListCtx> ApiClient::executeDeviceListRequest(bool allowReplay)
{
    _factory.clearQueryParameters();
    auto req = _factory.createRequest(api_ra_devices);
    req.setRawHeader("authorization", QStringLiteral("Bearer %1").arg(_tokenCtx.accessToken).toUtf8());
    auto reply = _rest.get(req);

    return execRequest<DeviceListCtx>(std::move(reply), [](const std::optional<QJsonDocument>& doc, int status) {
        DeviceListCtx ctx;
        ctx.res.status = status;

        if (doc && !doc->isNull()) {
            if (status == 200 && doc->isArray()) {
                for (const auto& item : doc->array()) {
                    Device d;
                    d.seagateDeviceID = item[jkey_seagateDeviceID].toString();
                    d.certificateCommonName = item[jkey_certificateCommonName].toString();
                    d.setFriendlyName(item[jkey_friendlyName].toString());
                    d.hostname = item[jkey_hostname].toString();
                    ctx.deviceList.addDevice(d);
                }
            }
            else {
                ctx.res.errorString = (*doc)[jkey_name].toString();
                ctx.res.errorStacktrace = (*doc)[jkey_stacktrace].toString();
            }
            qCDebug(lcDeviceApiClient) << doc;
        }
        return ctx;
    }).then(this, [this, allowReplay](const DeviceListCtx& ctx) {
        if (!allowReplay || (ctx.res.status != 401 && ctx.res.status != 403)) {
            return QtFuture::makeReadyValueFuture(ctx);
        }

        qCWarning(lcDeviceApiClient) << "[ra_device_list] auth failure, trying single refresh + replay" << ctx.res.status;
        return refreshAccessToken(true).then(this, [this](const TokenContext& refreshCtx) {
            if (refreshCtx.res.status != 200) {
                return QtFuture::makeReadyValueFuture(makeDeviceListError(refreshCtx));
            }
            return executeDeviceListRequest(false);
        }).unwrap();
    }).unwrap();
}

QFuture<DevicePathListCtx> ApiClient::executeDeviceInfoRequest(const QString& deviceId, bool allowReplay)
{
    _factory.clearQueryParameters();
    auto req = _factory.createRequest(QStringLiteral("%1%2").arg(api_ra_device_info).arg(deviceId));
    req.setRawHeader("authorization", QStringLiteral("Bearer %1").arg(_tokenCtx.accessToken).toUtf8());
    auto reply = _rest.get(req);

    return execRequest<DevicePathListCtx>(std::move(reply), [](const std::optional<QJsonDocument>& doc, int status) {
        DevicePathListCtx ctx;
        ctx.res.status = status;
        if (doc && !doc->isNull()) {
            if (status == 200) {
                const auto& paths = (*doc)[jkey_paths].toArray();
                for (const auto p: paths) {
                    DevicePath dpath(p[jkey_address].toString(), DevHelpers::strToDevType(p[jkey_type].toString()), DeviceOrigin::Remote, p[jkey_port].toInt());
                    ctx.devicePathList.append(dpath);
                }
            }
            else {
                ctx.res.errorString = (*doc)[jkey_name].toString();
                ctx.res.errorStacktrace = (*doc)[jkey_stacktrace].toString();
            }
        }
        return ctx;
    }).then(this, [this, deviceId, allowReplay](const DevicePathListCtx& ctx) {
        if (!allowReplay || (ctx.res.status != 401 && ctx.res.status != 403)) {
            return QtFuture::makeReadyValueFuture(ctx);
        }

        qCWarning(lcDeviceApiClient) << "[ra_device_info] auth failure, trying single refresh + replay" << ctx.res.status;
        return refreshAccessToken(true).then(this, [this, deviceId](const TokenContext& refreshCtx) {
            if (refreshCtx.res.status != 200) {
                return QtFuture::makeReadyValueFuture(makeDevicePathListError(refreshCtx));
            }
            return executeDeviceInfoRequest(deviceId, false);
        }).unwrap();
    }).unwrap();
}

bool ApiClient::isAccessTokenValid() const
{
    return !_tokenCtx.accessToken.isEmpty() && _tokenCtx.accessTokenExpireTime > QDateTime::currentDateTime();
}

TokenContext ApiClient::parseTokenContext(const std::optional<QJsonDocument> &doc, int status)
{
    qCDebug(lcDeviceApiClient) << "status:" << status;
    TokenContext ctx;
    ctx.res.status = status;
    if (doc && !doc->isNull()) {
        if (status == 200) {
            ctx.res.errorString.clear();
            ctx.accessToken = (*doc)[jkey_accessToken].toString();
            ctx.refreshToken = (*doc)[jkey_refreshToken].toString();
            ctx.tokenType = (*doc)[jkey_tokenType].toString();
            ctx.expiresIn = (*doc)[jkey_expiresIn].toInt();
            if (ctx.expiresIn > 0)
                ctx.accessTokenExpireTime = QDateTime::currentDateTime().addSecs(ctx.expiresIn);
        }
        else {
            ctx.res.errorString = (*doc)[jkey_name].toString();
            ctx.res.errorStacktrace = (*doc)[jkey_stacktrace].toString();
            qCDebug(lcDeviceApiClient) << "error string:" << ctx.res.errorString;
            if (ctx.res.errorString == QStringLiteral("invalid refresh token"))
                ctx.refreshToken.clear();
        }
        _tokenCtx = ctx;
    }
    return ctx;
}
