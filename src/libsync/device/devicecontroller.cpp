#include "devicecontroller.h"
#include "deviceaggregator.h"
#include "configfile.h"
#include "deviceapi.h"
#include "devicepathresolver.h"
#include "device/mdnsclient.h"

#include <QLoggingCategory>
#include <QtConcurrent>

Q_LOGGING_CATEGORY(lcDeviceController, "device.controller", QtDebugMsg)

namespace {

void cleanupEmptyCN(QList<DevicePath>& paths)
{
    auto it = std::remove_if(paths.begin(), paths.end(), [](DevicePath& devPath) {
        if (devPath.about.certificate_common_name.isEmpty()) {
            return true;
        }
        return false;
    });
    paths.erase(it, paths.end());
}

QList<DevicePath> nonRemotePaths(const QList<DevicePath>& paths)
{
    return Device::nonRemotePaths(paths);
}

void replaceAccountRemotePaths(QList<DevicePath>& allPaths, const QList<DevicePath>& remotePaths)
{
    allPaths = Device::replaceRemotePaths(allPaths, remotePaths);
}

}

DeviceController::DeviceController(QObject *parent)
    : QObject(parent)
    , _api(new ApiClient(this))
    , _devApi(new DeviceApi(this))
    , _mdns(new MdnsClient(this))
    , _aggregator(new DeviceAggregator(this))
    , _pathResolver(new DevicePathResolver(_devApi, [this](const QString& deviceId) {
        return queryDeviceInfo(deviceId);
    }, this))
{
    qRegisterMetaType<DevicePath>("DevicePath");
    qRegisterMetaType<Device>("Device");
    qRegisterMetaType<DeviceInfoAbout>("DeviceInfoAbout");
    qRegisterMetaType<DeviceInfoStatus>("DeviceInfoStatus");
    qRegisterMetaType<DevicePathResolutionResult>("DevicePathResolutionResult");

    connect(_mdns, &MdnsClient::resultsChanged, this, [this](const QList<DevicePath>& records) {
        qCDebug(lcDeviceController) << "mDNS updated, paths found:";
        qCDebug(lcDeviceController) << records;

        if (records.isEmpty()) {
            qCDebug(lcDeviceController) << "mDNS records not found, finish";
            // No mDNS records found, emit empty list
            mdns_finished.store(true);
            check_finished();
            return;
        }

        _devApi->query_about_all(records)
            .then(this, [this](const QList<DevicePath>& paths) {

                QList<DevicePath> devPaths(paths);
                cleanupEmptyCN(devPaths);
                for (auto& it: devPaths) {
                    it.deviceType = DeviceType::Local;
                    it.origin = DeviceOrigin::MDNS;
                }

                if (!devPaths.isEmpty()) {
                    // _aggregator->updateSource(DeviceOrigin::MDNS, devPaths);
                    for (auto& it: devPaths) {
                        it.origin = DeviceOrigin::MDNS;
                        it.deviceType = DeviceType::Local;
                    }
                    _aggregator->add_paths(devPaths);
                }

                allAccountPaths.append(devPaths);

                qCDebug(lcDeviceController) << "mDNS paths" << devPaths;

                mdns_finished.store(true);
                check_finished();
            });
    });

    connect(_aggregator, &DeviceAggregator::listUpdated, this, [this] {
        qCDebug(lcDeviceController) << "Aggregator path list updated";
        if (_aggregator) {
            QWriteLocker locker(&lock_);
            _mdnsDeviceList = DeviceAggregator::mergeDevices(_mdnsDeviceList, DeviceAggregator::build_devices(_aggregator->paths()));
        }
    });
}

void DeviceController::setEmail(const QString &email)
{
    qCDebug(lcDeviceController) << "set email" << email;
    _email = email;
    APP::ConfigFile cf;
    _api->setClientId(cf.clientId());
    _api->setRefreshToken(cf.refreshTokenForEmail(_email));
}

void DeviceController::start_new_account()
{
    if (_email.isEmpty()) {
        qCWarning(lcDeviceController) << "Invalid (empty) email";
        return;
    }

    mdns_finished.store(false);
    ra_finished.store(false);
    _mdnsDeviceList.clear();
    _raDeviceList.clear();
    _aggregator->clearAll();

    // Delete old MDNS devices
    // cleanupMDNS(_deviceList);
    _mdns->start();

    if (hasRefreshToken()) {
        qCDebug(lcDeviceController) << "Refresh token exist, requesting RA...";
        processQueryDeviceList();
    }
    else {
        qCDebug(lcDeviceController) << "No refresh token available, using mDNS only...";
        // Can't receive RA records, emit empty list
        ra_finished.store(true);
        check_finished();
    }
}

void DeviceController::prepareLogin(Device &dev)
{
    qCDebug(lcDeviceController) << "Prepare login started, device" << dev.toStringShort();

    currentDevice = dev;

    if (currentDevice.isStatic) {
        qCDebug(lcDeviceController) << "Static device, prepare login skipped";
        emit prepareLoginFinished(currentDevice);
        return;
    }

    auto fillAbout = [this] {
        qCDebug(lcDeviceController) << "fillAbout paths:";
        qCDebug(lcDeviceController) << currentDevice.paths;

        _devApi->query_about_all(currentDevice.paths)
            .then(this, [this](const QList<DevicePath>& ctx) {

                qCDebug(lcDeviceController) << "Prepare login query about finished";
                currentDevice.paths = ctx;

                qCDebug(lcDeviceController) << "fillAbout updated about paths:";
                qCDebug(lcDeviceController) << currentDevice.paths;

                _devApi->query_status_all(currentDevice.paths)
                    .then(this, [this](const QList<DevicePath>& ctx) {
                        qCDebug(lcDeviceController) << "Prepare login query status finished";
                        currentDevice.paths = ctx;

                        qCDebug(lcDeviceController) << "fillAbout updated status paths:";
                        qCDebug(lcDeviceController) << ctx;

                        emit prepareLoginFinished(currentDevice);
                    })
                    .onFailed(this, [this](const std::exception& e) {
                        qCWarning(lcDeviceController) << "Prepare login status exception:" << e.what();
                        emit prepareLoginFinished(currentDevice);
                    });
            })
            .onFailed(this, [this](const std::exception& e) {
                qCWarning(lcDeviceController) << "Prepare login about exception:" << e.what();
                emit prepareLoginFinished(currentDevice);
            });
    };

    if (dev.paths.isEmpty() && !currentDevice.seagateDeviceID.isEmpty()) {
        queryDeviceInfo(currentDevice.seagateDeviceID)
            .then(this, [this,fillAbout](const DevicePathListCtx& ctx) {
                qCDebug(lcDeviceController) << "prepareLogin, device info status code" << ctx.res.status << ctx.res.errorString;
                if (ctx.res.status == 200) {
                    currentDevice.updateRemotePathCache(ctx.devicePathList);
                    fillAbout();
                }
                else {
                    emit prepareLoginFinished(currentDevice);
                }
            });
    }
    else {
        fillAbout();
    }

}

// Plan:
// - start mDNS search
// - query RA for device list
//   - ask access code if no token, emits accessCodeRequest()
//   - then GUI have to handle code access dialog and call enterAccessCodeFromAccount
//     to continue device update.
//     enterAccessCodeFromAccount emits accessCodeResult
//     then call account_update_device_continue
// - merge paths for account device + in RA list (by CN)
// - query RA for device info if deviceID is not empty
// - query /about for each address found
// - wait RA and mDNS completed
// - build new device path from the list selecting by deviceCN
// - query /status for selected paths
// - emit account_update_device_finished when complete
void DeviceController::account_update_device(const Device& dev)
{
    qCDebug(lcDeviceController) << dev;
    loadRefreshToken();
    currentDevice = dev;
    mdns_finished.store(false);
    ra_finished.store(false);
    allAccountPaths = nonRemotePaths(currentDevice.paths);
    _mdns->start();

    // all ok, RA and mDNS lists are ready
    connect(this, &DeviceController::devices_updated, this, [this, devCN = dev.certificateCommonName] {

        qCDebug(lcDeviceController) << "devices_updated, paths:";
        qCDebug(lcDeviceController) << allAccountPaths;

        QHash<QString,DevicePath> uniques;
        for (const auto& path: std::as_const(allAccountPaths)) {
            if (path.about.certificate_common_name == devCN) {
                const auto key = QStringLiteral("%1:%2").arg(path.address).arg(path.port);
                uniques[key] = path;
            }
        }

        const auto& list = uniques.values();

        qCDebug(lcDeviceController) << "Devices for" << devCN << ":";
        qCDebug(lcDeviceController) << list;

        if (!list.isEmpty()) {
            _devApi->query_status_all(uniques.values())
                .then(this, [this,devCN](QList<DevicePath> ctx){
                    qCDebug(lcDeviceController) << "Path status for" << devCN << ":";
                    qCDebug(lcDeviceController) << ctx;
                    currentDevice.paths = DeviceList::mergePaths(nonRemotePaths(currentDevice.paths), ctx);
                    emit account_update_device_finished(currentDevice);
                });
        }
        else {
            emit account_update_device_finished(currentDevice);
        }

    }, Qt::SingleShotConnection);

    qCDebug(lcDeviceController) << "Query device list";
    queryDeviceList()
        .then(this, [this, d=dev](const DeviceListCtx& ctx) {
            qCDebug(lcDeviceController) << "DeviceListCtx" << ctx.res.status;

            if (ctx.res.status != 200) {
                initAccessCode();
                return;
            }

            auto finishTask = [this]() {
                ra_finished.store(true);
                check_finished();
            };

            const auto dev_ra = ctx.deviceList.find_by_cn(d.certificateCommonName);

            if (!dev_ra || dev_ra->seagateDeviceID.isEmpty()) {
                qCDebug(lcDeviceController) << "Device not found or ID empty (CN lookup)";
                finishTask();
                return;
            }

            currentDevice.seagateDeviceID = dev_ra->seagateDeviceID;
            if (currentDevice.friendlyName().isEmpty() && !dev_ra->friendlyName().isEmpty()) {
                currentDevice.setFriendlyName(dev_ra->friendlyName());
            }
            if (currentDevice.hostname.isEmpty()) {
                currentDevice.hostname = dev_ra->hostname;
            }

            qCDebug(lcDeviceController) << "Query device info for" << dev_ra->seagateDeviceID;
            queryDeviceInfo(dev_ra->seagateDeviceID)
                .then(this, [this, finishTask, id=dev_ra->seagateDeviceID](const DevicePathListCtx& ctx) {
                    qCDebug(lcDeviceController) << "acc update ra_device_info code" << ctx.res.status << ctx.res.errorString;

                    if (ctx.res.status == 200) {
                        currentDevice.updateRemotePathCache(ctx.devicePathList);
                        _devApi->query_about_all(currentDevice.remotePaths()).then(this, [this,finishTask](const QList<DevicePath>& paths) {
                            qCDebug(lcDeviceController) << "About updated:" << paths;

                            currentDevice.updateRemotePathCache(paths);
                            replaceAccountRemotePaths(allAccountPaths, currentDevice.remotePaths());
                            finishTask();
                        });
                    }
                    else if (ctx.res.status == 404) {
                        // Device not found
                        qCDebug(lcDeviceController) << "Device not found" << id;
                        finishTask();
                    }
                    else {
                        initAccessCode();
                    }
                });
    });

}

void DeviceController::account_update_device_continue(std::optional<Device> dev)
{
    if (!dev) {
        qCDebug(lcDeviceController) << "acc update continue: no account device";
        ra_finished.store(true);
        check_finished();
        return;
    }

    if (dev->seagateDeviceID.isEmpty()) {
        qCDebug(lcDeviceController) << "acc update continue: device has no ID";
        ra_finished.store(true);
        check_finished();
        return;
    }

    currentDevice = *dev;

    queryDeviceInfo(dev->seagateDeviceID)
        .then(this, [this](const DevicePathListCtx& ctx) {
            qCDebug(lcDeviceController) << "acc update continue ra_device_info code" << ctx.res.status << ctx.res.errorString;
            if (ctx.res.status == 200) {
                currentDevice.updateRemotePathCache(ctx.devicePathList);

                _devApi->query_about_all(currentDevice.remotePaths())
                    .then(this, [this](const QList<DevicePath>& paths) {
                        qCDebug(lcDeviceController) << "Paths added:";
                        qCDebug(lcDeviceController) << paths;
                        currentDevice.updateRemotePathCache(paths);
                        replaceAccountRemotePaths(allAccountPaths, currentDevice.remotePaths());
                        ra_finished.store(true);
                        check_finished();
                    });

            }
            else {
                qCDebug(lcDeviceController) << "acc update continue ra_device_info fail";
                ra_finished.store(true);
                check_finished();
            }
        });
}


void DeviceController::force_ra_account()
{
    mdns_finished.store(false);
    ra_finished.store(false);

    _mdns->start();
    _mdnsDeviceList.clear();
    _raDeviceList.clear();
    _aggregator->clearAll();
    force_device_list_request = true;
    forceQueryDeviceList();
}

void DeviceController::setTokenContext(TokenContext &&tokenCtx)
{
    _api->setTokenCtx(std::move(tokenCtx));
}

bool DeviceController::hasRefreshToken() const
{
    return _api->hasRefreshToken();
}

void DeviceController::saveRefreshToken()
{
    APP::ConfigFile cf;
    cf.setRefreshTokenForEmail(_api->tokenCtx().refreshToken, _email);
    qCDebug(lcDeviceController) << "Refresh token save, token exist" << !_api->tokenCtx().refreshToken.isEmpty();
}

void DeviceController::loadRefreshToken()
{
    APP::ConfigFile cf;
    const auto& token = cf.refreshTokenForEmail(_email);
    _api->setRefreshToken(token);
    qCDebug(lcDeviceController) << "Refresh token load, token exist" << !token.isEmpty();
}

DeviceList DeviceController::getDevices() const
{
    QReadLocker locker(&lock_);

    return DeviceAggregator::mergeDevices(_mdnsDeviceList, _raDeviceList);
}

QFuture<DeviceListCtx> DeviceController::queryDeviceList()
{
    loadRefreshToken();
    return _api->ra_device_list();
}

QFuture<DevicePathListCtx> DeviceController::queryDeviceInfo(const QString &deviceId)
{
    loadRefreshToken();
    return _api->ra_device_info(deviceId);
}

QFuture<DevicePathResolutionResult> DeviceController::resolveDevicePath(const Device& device)
{
    if (!_pathResolver) {
        DevicePathResolutionResult result;
        result.device = device;
        return QtFuture::makeReadyValueFuture(result);
    }

    return _pathResolver->resolve(device);
}

void DeviceController::initAccessCode()
{
    qCDebug(lcDeviceController) << "initAccessCode";
    _api->clearTokens();
    _api->ra_initiate(_email)
        .then(this, [this](const InitContext& ctx) {
            qCWarning(lcDeviceController) << "initAccessCode result" << ctx.res.status << ctx.res.errorString;
            if (ctx.res.status == 200) {
                emit accessCodeRequest();
            }
            else {
                emit accessCodeResult(AccessCodeContext::Init, ctx.res.status, ctx.res.errorString, ctx.res.errorStacktrace);
            }
        });
}

void DeviceController::enterAccessCode(const QString &code, bool from_account)
{
    qCDebug(lcDeviceController) << "enterAccessCode, from account:" << from_account;
    _api->ra_token(code)
        .then(this, [this,from_account](const TokenContext& ctx) {
            qCDebug(lcDeviceController) << "enterAccessCode" << ctx.res.status << ctx.res.errorString;
            if (ctx.res.status == 200) {
                saveRefreshToken();
                emit accessCodeResult(AccessCodeContext::Token, ctx.res.status, {}, {});
                if (!from_account) {
                    if (force_device_list_request) {
                        qCDebug(lcDeviceController) << "force RA mode, queued forceQueryDeviceList call";
                        // drop call stack
                        QMetaObject::invokeMethod(this, &DeviceController::forceQueryDeviceList, Qt::QueuedConnection);
                    }
                }
            }
            else {
                emit accessCodeResult(AccessCodeContext::Token, ctx.res.status, ctx.res.errorString, ctx.res.errorStacktrace);
            }
        });
}

void DeviceController::processQueryDeviceList()
{
    qCDebug(lcDeviceController) << "Query device list";
    queryDeviceList()
        .then(this, [this](const DeviceListCtx& ctx) {
            qCDebug(lcDeviceController) << "DeviceListCtx" << ctx.res.status;
            if (ctx.res.status == 200) {
                {
                    QWriteLocker locker(&lock_);
                    _raDeviceList = DeviceAggregator::mergeDevices(_raDeviceList, ctx.deviceList);
                }
                qCDebug(lcDeviceController) << "Device list:";
                qCDebug(lcDeviceController) << ctx.deviceList;
                saveRefreshToken();
            }
            ra_finished.store(true);
            check_finished();
        });
}

void DeviceController::forceQueryDeviceList()
{
    qCDebug(lcDeviceController) << "forceQueryDeviceList";
    queryDeviceList()
        .then(this, [this](const DeviceListCtx& ctx) {
            qCDebug(lcDeviceController) << "DeviceListCtx" << ctx.res.status;
            if (ctx.res.status == 200) {
                {
                    QWriteLocker locker(&lock_);
                    _raDeviceList = DeviceAggregator::mergeDevices(_raDeviceList, ctx.deviceList);
                    // _deviceList = ctx.deviceList;
                }

                qCDebug(lcDeviceController) << "Device list:";
                qCDebug(lcDeviceController) << ctx.deviceList;
                saveRefreshToken();
                ra_finished.store(true);
                check_finished();
            }
            else {
                initAccessCode();
            }
        });
}

void DeviceController::check_finished()
{
    bool m = mdns_finished.load();
    bool r = ra_finished.load();

    qCDebug(lcDeviceController) << "mdns finished" << m << "ra finished" << r;
    if (m && r) {
        emit devices_updated(hasRefreshToken());
        force_device_list_request = false;
        mdns_finished.store(false);
        ra_finished.store(false);
    }
}
