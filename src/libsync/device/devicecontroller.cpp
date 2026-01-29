#include "devicecontroller.h"
#include "deviceaggregator.h"
#include "configfile.h"
#include "deviceapi.h"
#include "device/mdnsclient.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcDeviceController, "device.controller", QtDebugMsg)

namespace {

// void cleanupMDNS(QList<Device>& deviceList)
// {
//     auto it = std::remove_if(deviceList.begin(), deviceList.end(), [](Device& dev) {
//         if (dev.origin == DeviceOrigin::MDNS) {
//             return true;
//         }

//         auto pathIt = std::remove_if(dev.paths.begin(), dev.paths.end(), [](const DevicePath& p) {
//             return p.origin == DeviceOrigin::MDNS;
//         });
//         dev.paths.erase(pathIt, dev.paths.end());

//         return false;
//     });

//     deviceList.erase(it, deviceList.end());
// }

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

}

DeviceController::DeviceController(QObject *parent)
    : QObject(parent)
    , _api(new ApiClient(this))
    , _devApi(new DeviceApi(this))
    , _mdns(new MdnsClient(this))
    , _aggregator(new DeviceAggregator(this))
{
    qRegisterMetaType<DevicePath>("DevicePath");
    qRegisterMetaType<Device>("Device");
    qRegisterMetaType<DeviceInfoAbout>("DeviceInfoAbout");
    qRegisterMetaType<DeviceInfoStatus>("DeviceInfoStatus");

    connect(_mdns, &MdnsClient::resultsChanged, this, [this](const QList<DevicePath>& records) {
        qCDebug(lcDeviceController) << "mDNS updated, paths found:";
        qCDebug(lcDeviceController) << records;

        if (records.isEmpty()) {
            qCDebug(lcDeviceController) << "mDNS records not found, finish";
            // No mDNS records found, emit empty list
            mdns_finished = true;
            check_finished();
            return;
        }

        _devApi->query_about_all(records)
            .then(this, [this](const QList<DevicePath>& paths) {

                QList<DevicePath> devPaths(paths);
                cleanupEmptyCN(devPaths);

                if (!devPaths.isEmpty()) {
                    _aggregator->updateSource(DeviceOrigin::MDNS, devPaths);
                }

                allAccountPaths.append(devPaths);

                qCDebug(lcDeviceController) << "mDNS paths" << devPaths;

                mdns_finished = true;
                check_finished();
            });
    });

    connect(_aggregator, &DeviceAggregator::listUpdated, this, [this] {
        qCDebug(lcDeviceController) << "Aggregator path list updated";
        if (_aggregator) {
            QWriteLocker locker(&lock_);
            _deviceList = DeviceAggregator::build_devices(_aggregator->getDevicePaths());
        }
    });
}

void DeviceController::setEmail(const QString &email)
{
    qCDebug(lcDeviceController) << "set email" << email;
    _email = email;
    CUR::ConfigFile cf;
    _api->setClientId(cf.clientId());
    _api->setRefreshToken(cf.refreshTokenForEmail(_email));
}

void DeviceController::start_new_account()
{
    if (_email.isEmpty()) {
        qCWarning(lcDeviceController) << "Invalid (empty) email";
        return;
    }

    mdns_finished = false;
    ra_finished = false;
    _deviceList.clear();
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
        ra_finished = true;
        check_finished();
    }
}

void DeviceController::prepareLogin(Device &dev)
{
    qCDebug(lcDeviceController) << "Prepare login started, device" << dev.toStringShort();

    currentDevice = dev;

    // if (dev.paths.isEmpty()) {
    //     qCWarning(lcDeviceController) << "No path in device";
    //     emit prepareLoginFinished(currentDevice);
    //     return;
    // }

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

    if (dev.paths.isEmpty()) {
        _api->ra_device_info(currentDevice.seagateDeviceID)
            .then(this, [this,fillAbout](const DevicePathListCtx& ctx) {
                qCDebug(lcDeviceController) << "prepareLogin, device info status code" << ctx.res.status << ctx.res.errorString;
                if (ctx.res.status == 200) {
                    currentDevice.paths = ctx.devicePathList;
                    fillAbout();
                }
            });
    }
    else {
        fillAbout();
    }

}

// Plan:
// - start mDNS search
// - query RA for device info
//   - ask access code if no token, emits accessCodeRequest()
//   - then GUI have to handle code access dialog and call enterAccessCodeFromAccount
//     to continue device update.
//     enterAccessCodeFromAccount emits accessCodeResult
//     then call account_update_device_continue
// - query /about for each address found
// - wait RA and mDNS completed
// - build new device path from the list selecting by deviceCN
// - query /status for selected paths
// - emit account_update_device_finished when complete
void DeviceController::account_update_device(const Device& dev)
{
    qCDebug(lcDeviceController) << "[account_update_device]";
    loadRefreshToken();
    mdns_finished = false;
    ra_finished = false;
    allAccountPaths.clear();
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
                    emit account_update_device_finished(ctx);
                });
        }
        else {
            emit account_update_device_finished({});
        }

    }, Qt::SingleShotConnection);

    qDebug() << "Query device info for" << dev.seagateDeviceID;
    _api->ra_device_info(dev.seagateDeviceID)
        .then(this, [this,dev](const DevicePathListCtx& ctx) {
            qCDebug(lcDeviceController) << "acc update ra_device_info code" << ctx.res.status << ctx.res.errorString;

            if (ctx.res.status == 200) {

                _devApi->query_about_all(ctx.devicePathList)
                    .then(this, [this](const QList<DevicePath>& paths) {

                        qCDebug(lcDeviceController) << "About updated:";
                        qCDebug(lcDeviceController) << paths;

                        allAccountPaths.append(paths);
                        ra_finished = true;
                        check_finished();
                    });

            }
            else if (ctx.res.status == 404) {
                // Device not found
                qCDebug(lcDeviceController) << "Device not found" << dev.seagateDeviceID;
                ra_finished = true;
                check_finished();
            }
            else {
                initAccessCode();
            }
        });
}

void DeviceController::account_update_device_continue(const Device &dev)
{
    _api->ra_device_info(dev.seagateDeviceID)
        .then(this, [this](const DevicePathListCtx& ctx) {
            qCDebug(lcDeviceController) << "acc update continue ra_device_info code" << ctx.res.status << ctx.res.errorString;
            if (ctx.res.status == 200) {

                _devApi->query_about_all(ctx.devicePathList)
                    .then(this, [this](const QList<DevicePath>& paths) {
                        qCDebug(lcDeviceController) << "Paths added:";
                        qCDebug(lcDeviceController) << paths;
                        allAccountPaths.append(paths);
                        ra_finished = true;
                        check_finished();
                    });

            }
            else {
                qCDebug(lcDeviceController) << "acc update continue ra_device_info fail";
                ra_finished = true;
                check_finished();
            }
        });
}


void DeviceController::evaluateDeviceStatus(Device *dev)
{
    qCDebug(lcDeviceController) << "Starting device path checks";
    if (isEvaluationRunning()) {
        qCWarning(lcDeviceController) << "Already running";
        emit evaluate_finished();
    }

    _isEvaluationRunning = true;

    _devApi->query_status_all(dev->paths)
        .then(this, [this,dev](const QList<DevicePath>& ctx) {
            _isEvaluationRunning = false;
            dev->paths = ctx;
            qCInfo(lcDeviceController) << "Evaluating finished, paths:";
            qCDebug(lcDeviceController) << dev->paths;
            emit evaluate_finished();
        });
}

bool DeviceController::isEvaluationRunning() const
{
    return _isEvaluationRunning;
}

void DeviceController::force_ra_account()
{
    mdns_finished = false;
    ra_finished = false;

    _mdns->start();
    _deviceList.clear();
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
    CUR::ConfigFile cf;
    cf.setRefreshTokenForEmail(_api->tokenCtx().refreshToken, _email);
    qCDebug(lcDeviceController) << "Refresh token save, token exist" << !_api->tokenCtx().refreshToken.isEmpty();
}

void DeviceController::loadRefreshToken()
{
    CUR::ConfigFile cf;
    const auto& token = cf.refreshTokenForEmail(_email);
    _api->setRefreshToken(token);
    qCDebug(lcDeviceController) << "Refresh token load, token exist" << !token.isEmpty();
}

QList<Device> DeviceController::getDevices() const
{
    QReadLocker locker(&lock_);
    return _deviceList;
}

std::optional<Device> DeviceController::getDevice(const QString &deviceCN) const
{
    const auto& it = std::find_if(_deviceList.cbegin(), _deviceList.cend(), [deviceCN](const Device& d) {
        return d.certificateCommonName == deviceCN;
    });

    if (it != _deviceList.cend())
        return *it;

    return std::nullopt;
}

QFuture<DevicePathListCtx> DeviceController::queryDeviceInfo(const QString &deviceId)
{
    return _api->ra_device_info(deviceId);
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
                emit accessCodeResult(AccessCodeResult::Error, ctx.res.errorString, ctx.res.errorStacktrace);
            }
        });
}

void DeviceController::enterAccessCode(const QString &code)
{
    qCDebug(lcDeviceController) << "enterAccessCode";
    _api->ra_token(code)
        .then(this, [this](const TokenContext& ctx) {
            qCDebug(lcDeviceController) << "enterAccessCode" << ctx.res.status << ctx.res.errorString;
            if (ctx.res.status == 200) {
                saveRefreshToken();
                emit accessCodeResult(AccessCodeResult::Accepted, {}, {});
                if (force_device_list_request) {
                    qCDebug(lcDeviceController) << "force RA mode, queued forceQueryDeviceList call";
                    QTimer::singleShot(1, this, [this] {
                        forceQueryDeviceList();
                    });
                }
            }
            else {
                emit accessCodeResult(AccessCodeResult::Error, ctx.res.errorString, ctx.res.errorStacktrace);
            }
        });
}

void DeviceController::enterAccessCodeFromAccount(const QString &code)
{
    qCDebug(lcDeviceController) << "enterAccessCode";
    _api->ra_token(code)
        .then(this, [this](const TokenContext& ctx) {
            qCWarning(lcDeviceController) << "enterAccessCode" << ctx.res.status << ctx.res.errorString << ctx.res.errorStacktrace;
            if (ctx.res.status == 200) {
                saveRefreshToken();
                emit accessCodeResult(AccessCodeResult::Accepted, {}, {});
            }
            else {
                emit accessCodeResult(AccessCodeResult::Error, ctx.res.errorString, ctx.res.errorStacktrace);
            }
        });
}

QFuture<QList<DevicePath> > DeviceController::query_status_all(const Device &dev)
{
    return _devApi->query_status_all(dev.paths);
}

void DeviceController::processQueryDeviceList()
{
    qCDebug(lcDeviceController) << "Query device list";
    _api->ra_device_list()
        .then(this, [this](const DeviceListCtx& ctx) {
            qCDebug(lcDeviceController) << "DeviceListCtx" << ctx.res.status;
            if (ctx.res.status == 200) {
                {
                    QWriteLocker locker(&lock_);
                    _deviceList = ctx.deviceList;
                }
                qCDebug(lcDeviceController) << "Device list:";
                qCDebug(lcDeviceController) << ctx.deviceList;
                saveRefreshToken();
            }
            ra_finished = true;
            check_finished();
        });
}

void DeviceController::forceQueryDeviceList()
{
    qCDebug(lcDeviceController) << "forceQueryDeviceList";

    _api->ra_device_list()
        .then(this, [this](const DeviceListCtx& ctx) {
            qCDebug(lcDeviceController) << "DeviceListCtx" << ctx.res.status;
            if (ctx.res.status == 200) {
                {
                    QWriteLocker locker(&lock_);
                    _deviceList = ctx.deviceList;
                }

                qCDebug(lcDeviceController) << "Device list:";
                qCDebug(lcDeviceController) << ctx.deviceList;
                saveRefreshToken();
                ra_finished = true;
                // mdns_finished = true; // ignore mDNS here
                check_finished();
            }
            else {
                initAccessCode();
            }
        });
}

void DeviceController::check_finished()
{
    qCDebug(lcDeviceController) << "mdns ready" << mdns_finished << "ra ready" << ra_finished;
    if (mdns_finished && ra_finished) {
        emit devices_updated(hasRefreshToken());
        force_device_list_request = false;
    }
}
