#pragma once

#include "curatorlib.h"
#include "device/apiclient.h"

#include <QObject>
#include <QReadWriteLock>

class ApiClient;
class DeviceApi;
class MdnsClient;
class DeviceAggregator;

class APPLICATIONSYNC_EXPORT DeviceController: public QObject
{
    Q_OBJECT

public:
    enum class AccessCodeResult {
        Accepted,
        Error
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
    void account_update_device_continue(std::optional<Device> dev);

    void evaluateDeviceStatus(Device* dev);
    bool isEvaluationRunning() const;

    // When "Can't find device" command
    void force_ra_account();

    const TokenContext& tokenContext() const {return _api->tokenCtx();}
    void setTokenContext(TokenContext&& tokenCtx);

    bool hasRefreshToken() const;
    void saveRefreshToken();
    void loadRefreshToken();

    QList<Device> getDevices() const;

    QFuture<DevicePathListCtx> queryDeviceInfo(const QString& deviceId);

    void initAccessCode();
    void enterAccessCode(const QString& code);
    void enterAccessCodeFromAccount(const QString& code);

    QFuture<QList<DevicePath>> query_status_all(const Device& dev);

signals:
    void prepareLoginFinished(const Device& d);

    // raQueried == true if RA was queried
    // raQueried == false if no refresh token and RA isn't queried
    void devices_updated(bool raQueried);

    void evaluate_finished();

    void accessCodeRequest();
    void accessCodeResult(DeviceController::AccessCodeResult result, const QString& errorString, const QString& errorStacktrace);

    void account_update_device_finished(const QList<DevicePath>& paths);

protected:
    void processQueryDeviceList();
    void forceQueryDeviceList();
    void check_finished();

protected:
    ApiClient* _api = nullptr;
    DeviceApi* _devApi = nullptr;
    MdnsClient* _mdns = nullptr;
    DeviceAggregator* _aggregator = nullptr;
    QString _email;
    QList<Device> _deviceList;
    Device currentDevice;

    mutable QReadWriteLock lock_;

    //bool mdns_finished = false;
    //bool ra_finished = false;
    // 0 = none, 1 = mDNS ready, 2 = RA ready, 3 = All ready
    std::atomic<bool> mdns_finished{false};
    std::atomic<bool> ra_finished{false};

    bool force_device_list_request = false;
    bool _isEvaluationRunning = false;

    QList<DevicePath> allAccountPaths;
};
