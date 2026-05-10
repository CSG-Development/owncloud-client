#pragma once

#include "personalcloudlib.h"
#include "device/apiclient.h"
#include "device/devicepathresolver.h"

#include <QElapsedTimer>
#include <QObject>
#include <QReadWriteLock>

class ApiClient;
class DeviceApi;
class MdnsClient;
class DeviceAggregator;

namespace APP {
class CredentialManager;
}

class APPLICATIONSYNC_EXPORT DeviceController: public QObject
{
    Q_OBJECT

public:
    enum class AccessCodeContext {
        Init,
        Token
    };

    explicit DeviceController(QObject* parent = nullptr);

    void setEmail(const QString& email);

    void start_new_account();
    void prepareLogin(Device& dev);

    // Query RA to update device list and check status.
    // emits:
    // - account_update_device_finished
    // - accessCodeRequest
    // - accessCodeResult
    void account_update_device(const Device& dev);
    // Query mDNS only to refresh local account paths before path resolution.
    void account_update_local_paths(const Device& dev);
    void account_update_device_continue(std::optional<Device> dev);

    // When "Can't find device" command
    void force_ra_account();

    const TokenContext& tokenContext() const {return _api->tokenCtx();}
    void setTokenContext(TokenContext&& tokenCtx);

    bool hasRefreshToken() const;
    void saveRefreshToken();
    QFuture<void> loadRefreshToken();

    DeviceList getDevices() const;

    QFuture<DeviceListCtx> queryDeviceList();
    QFuture<DevicePathListCtx> queryDeviceInfo(const QString& deviceId);
    QFuture<DevicePathResolutionResult> resolveDevicePath(const Device& device, const std::optional<QUuid>& avoidPathId = std::nullopt,
        const std::optional<QUuid>& preferredPathId = std::nullopt);

    void initAccessCode();
    void enterAccessCode(const QString& code, bool from_account);

signals:
    void prepareLoginFinished(const Device& d);

    // raQueried == true if RA was queried
    // raQueried == false if no refresh token and RA isn't queried
    void devices_updated(bool raQueried);

    void accessCodeRequest();
    void accessCodeResult(DeviceController::AccessCodeContext context, int status_code, const QString& errorString, const QString& errorStacktrace);

    void account_update_device_finished(const Device& device);

protected:
    void processQueryDeviceList();
    void forceQueryDeviceList();
    void processAccountUpdateDeviceList();
    void finishAccountUpdateWithDiscoveredPaths(const QString& devCN);
    void startMdnsDiscovery();
    void finishMdnsDiscoveryPhase(quint64 generation);
    void check_finished();
    void startRaTiming(const QString& operation);
    void logRaTimingSummary(const QString& completionReason);
    void saveRefreshTokenToSecureStorage(const QString& email, const QString& token);
    void removeRefreshTokenFromSecureStorage(const QString& email);

protected:
    ApiClient* _api = nullptr;
    DeviceApi* _devApi = nullptr;
    MdnsClient* _mdns = nullptr;
    DeviceAggregator* _aggregator = nullptr;
    DevicePathResolver* _pathResolver = nullptr;
    APP::CredentialManager* _credentialManager = nullptr;
    QString _email;
    DeviceList _raDeviceList;
    DeviceList _mdnsDeviceList;
    Device currentDevice;

    mutable QReadWriteLock lock_;

    //bool mdns_finished = false;
    //bool ra_finished = false;
    // 0 = none, 1 = mDNS ready, 2 = RA ready, 3 = All ready
    std::atomic<bool> mdns_finished{false};
    std::atomic<bool> ra_finished{false};

    bool force_device_list_request = false;

    enum class DeferredRaQuery {
        None,
        NewAccount,
        ForceAccount,
        AccountUpdate,
        LocalAccountUpdate
    };
    DeferredRaQuery _deferredRaQuery = DeferredRaQuery::None;
    quint64 _mdnsDiscoveryGeneration = 0;
    quint64 _refreshTokenGeneration = 0;
    QString _refreshTokenLoadedEmail;
    struct RaTiming {
        bool active = false;
        QString operation;
        QElapsedTimer totalTimer;
        QElapsedTimer mdnsTimer;
        qint64 mdnsDiscoveryMs = -1;
        qint64 mdnsAboutFileServerMs = -1;
        qint64 remoteAccessDeviceListMs = -1;
        qint64 remoteAccessPathsMs = -1;
        qint64 remoteAccessInitMs = -1;
        qint64 remoteAccessTokenMs = -1;
        qint64 remoteAboutFileServerMs = -1;
        qint64 pathStatusFileServerMs = -1;
    };
    RaTiming _raTiming;

    QList<DevicePath> allAccountPaths;
};
