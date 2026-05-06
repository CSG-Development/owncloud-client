#include "devicecontroller.h"
#include "deviceaggregator.h"
#include "configfile.h"
#include "creds/credentialmanager.h"
#include "deviceapi.h"
#include "devicelogging.h"
#include "devicepathresolver.h"
#include "device/mdnsclient.h"

#include <QLoggingCategory>
#include <QPromise>
#include <QtConcurrent>

#include <memory>

Q_LOGGING_CATEGORY(lcDeviceController, "device.controller", QtDebugMsg)

namespace {

void cleanupEmptyCN(QList<DevicePath>& paths)
{
    QStringList droppedEndpoints;
    auto it = std::remove_if(paths.begin(), paths.end(), [](DevicePath& devPath) {
        if (devPath.about.certificate_common_name.isEmpty()) {
            return true;
        }
        return false;
    });
    for (auto dropped = it; dropped != paths.end(); ++dropped) {
        droppedEndpoints.append(QStringLiteral("%1:%2").arg(dropped->address, QString::number(dropped->port)));
    }
    paths.erase(it, paths.end());
    if (!droppedEndpoints.isEmpty()) {
        qCWarning(lcDeviceData).noquote()
            << "mdns_about dropped_records missing_cn"
            << droppedEndpoints.join(QStringLiteral(", "));
    }
}

QList<DevicePath> nonRemotePaths(const QList<DevicePath>& paths)
{
    return Device::nonRemotePaths(paths);
}

void replaceAccountRemotePaths(QList<DevicePath>& allPaths, const QList<DevicePath>& remotePaths)
{
    allPaths = Device::replaceRemotePaths(allPaths, remotePaths);
}

void applyRemoteAccessIdentity(Device& targetDevice, const Device& remoteDevice)
{
    targetDevice.seagateDeviceID = remoteDevice.seagateDeviceID;
    if (targetDevice.friendlyName().isEmpty() && !remoteDevice.friendlyName().isEmpty()) {
        targetDevice.setFriendlyName(remoteDevice.friendlyName());
    }
    if (targetDevice.hostname.isEmpty()) {
        targetDevice.hostname = remoteDevice.hostname;
    }
}

void logRemoteAccessIdentityApplied(const char* context, const Device& device)
{
    qCInfo(lcDeviceData).noquote()
        << context
        << "Remote Access identity applied."
        << QStringLiteral("{cn:%1,id:%2,friendly:%3,hostname:%4,pathCount:%5,remotePathCount:%6}")
               .arg(device.certificateCommonName, device.seagateDeviceID, device.friendlyName(), device.hostname,
                   QString::number(device.paths.size()), QString::number(device.remotePaths().size()));
}

void logRemoteAccessCnLookupMismatch(const char* context, const Device& localDevice, const DeviceList& remoteDevices)
{
    QStringList localPathCns;
    for (const auto& path : localDevice.paths) {
        if (!path.about.certificate_common_name.isEmpty()) {
            localPathCns.append(path.about.certificate_common_name);
        }
    }
    localPathCns.removeDuplicates();

    QStringList remoteSummaries;
    for (const auto& remoteDevice : remoteDevices.devices()) {
        remoteSummaries.append(QStringLiteral("{cn:%1,id:%2,friendly:%3,hostname:%4}")
            .arg(remoteDevice.certificateCommonName, remoteDevice.seagateDeviceID, remoteDevice.friendlyName(), remoteDevice.hostname));
    }

    qCWarning(lcDeviceData).noquote()
        << context
        << "Remote Access device merge failed by certificateCommonName."
        << "Local device:"
        << QStringLiteral("{cn:%1,hostname:%2,friendly:%3,pathAboutCNs:[%4]}")
               .arg(localDevice.certificateCommonName, localDevice.hostname, localDevice.friendlyName(), localPathCns.join(QStringLiteral(", ")))
        << "Remote Access devices:"
        << remoteSummaries.join(QStringLiteral(", "));
}

bool isRemoteAccessAuthFailure(int status)
{
    return status == 401 || status == 403;
}

bool requiresRemoteAccessPrompt(int status)
{
    return status == -2 || isRemoteAccessAuthFailure(status);
}

bool isInvalidRefreshTokenError(const ResultContext& result)
{
    return result.errorString.contains(QStringLiteral("invalid refresh token"), Qt::CaseInsensitive)
        || result.errorStacktrace.contains(QStringLiteral("invalid refresh token"), Qt::CaseInsensitive);
}

QString normalizedEmail(const QString& email)
{
    return email.trimmed().toLower();
}

QString refreshTokenCredentialKey(const QString& email)
{
    return QStringLiteral("RemoteAccess/RefreshToken/%1").arg(normalizedEmail(email));
}

QFuture<void> makeReadyVoidFuture()
{
    auto promise = std::make_shared<QPromise<void>>();
    auto future = promise->future();
    promise->finish();
    return future;
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
    , _credentialManager(new APP::CredentialManager(this))
{
    qRegisterMetaType<DevicePath>("DevicePath");
    qRegisterMetaType<Device>("Device");
    qRegisterMetaType<DeviceInfoAbout>("DeviceInfoAbout");
    qRegisterMetaType<DeviceInfoStatus>("DeviceInfoStatus");
    qRegisterMetaType<DevicePathResolutionResult>("DevicePathResolutionResult");

    connect(_mdns, &MdnsClient::resultsChanged, this, [this](const QList<DevicePath>& records) {
        const auto generation = _mdnsDiscoveryGeneration;
        qCInfo(lcDeviceController) << "mDNS discovery update"
                                   << "generation" << generation
                                   << "rawPathCount" << records.size();
        qCDebug(lcDeviceController) << "mDNS updated, paths found:";
        qCDebug(lcDeviceController) << records;

        if (records.isEmpty()) {
            qCDebug(lcDeviceController) << "mDNS records not found, finish";
            // No mDNS records found, emit empty list
            finishMdnsDiscoveryPhase(generation);
            return;
        }

        _devApi->query_about_all(records)
            .then(this, [this, generation](const QList<DevicePath>& paths) {

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

                qCInfo(lcDeviceController) << "mDNS discovery about completed"
                                           << "generation" << generation
                                           << "usablePathCount" << devPaths.size();
                qCDebug(lcDeviceController) << "mDNS paths" << devPaths;

                finishMdnsDiscoveryPhase(generation);
            })
            .onFailed(this, [this, generation](const std::exception& e) {
                qCWarning(lcDeviceController) << "mDNS about query exception, finishing local discovery" << e.what();
                finishMdnsDiscoveryPhase(generation);
            });
    });

    connect(_aggregator, &DeviceAggregator::listUpdated, this, [this] {
        qCDebug(lcDeviceController) << "Aggregator path list updated";
        if (_aggregator) {
            QWriteLocker locker(&lock_);
            _mdnsDeviceList = DeviceAggregator::mergeDevices(_mdnsDeviceList, DeviceAggregator::build_devices(_aggregator->paths()));
            qCInfo(lcDeviceController) << "mDNS device list updated"
                                       << "deviceCount" << _mdnsDeviceList.devices().size();
        }
    });
}

void DeviceController::setEmail(const QString &email)
{
    qCDebug(lcDeviceController) << "set email" << email;
    _email = email;
    ++_refreshTokenGeneration;
    _refreshTokenLoadedEmail.clear();
    APP::ConfigFile cf;
    _api->setClientId(cf.clientId());
    _api->setRefreshToken({});
    loadRefreshToken();
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
    allAccountPaths.clear();
    _deferredRaQuery = DeferredRaQuery::None;

    // Delete old MDNS devices
    // cleanupMDNS(_deviceList);

    const auto requestEmail = normalizedEmail(_email);
    const auto tokenGeneration = _refreshTokenGeneration;
    loadRefreshToken().then(this, [this, requestEmail, tokenGeneration] {
        if (requestEmail != normalizedEmail(_email) || tokenGeneration != _refreshTokenGeneration) {
            qCDebug(lcDeviceController) << "Ignoring stale Remote Access startup continuation for email" << requestEmail;
            return;
        }

        if (hasRefreshToken()) {
            qCDebug(lcDeviceController) << "Refresh token exist, RA discovery deferred until mDNS completes";
            if (mdns_finished.load()) {
                processQueryDeviceList();
            } else {
                _deferredRaQuery = DeferredRaQuery::NewAccount;
            }
        }
        else {
            qCDebug(lcDeviceController) << "No refresh token available, using mDNS only after local discovery completes...";
            // Can't receive RA records, emit empty list
            ra_finished.store(true);
            _deferredRaQuery = DeferredRaQuery::None;
            if (mdns_finished.load()) {
                check_finished();
            }
        }
    });
    startMdnsDiscovery();
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
            })
            .onFailed(this, [this](const std::exception& e) {
                qCWarning(lcDeviceController) << "Prepare login device info exception:" << e.what();
                emit prepareLoginFinished(currentDevice);
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
    currentDevice = dev;
    mdns_finished.store(false);
    ra_finished.store(false);
    allAccountPaths = nonRemotePaths(currentDevice.paths);

    // all ok, RA and mDNS lists are ready
    connect(this, &DeviceController::devices_updated, this, [this, devCN = dev.certificateCommonName] {
        finishAccountUpdateWithDiscoveredPaths(devCN);
    }, Qt::SingleShotConnection);

    qCDebug(lcDeviceController) << "RA account update deferred until mDNS completes";
    _deferredRaQuery = DeferredRaQuery::AccountUpdate;
    startMdnsDiscovery();
}

void DeviceController::account_update_local_paths(const Device& dev)
{
    qCDebug(lcDeviceController) << "Local account path update" << dev;
    currentDevice = dev;
    mdns_finished.store(false);
    ra_finished.store(true);
    allAccountPaths = nonRemotePaths(currentDevice.paths);

    _deferredRaQuery = DeferredRaQuery::LocalAccountUpdate;
    startMdnsDiscovery();
}

void DeviceController::processAccountUpdateDeviceList()
{
    qCDebug(lcDeviceController) << "Query device list";
    queryDeviceList()
        .then(this, [this, d=currentDevice](const DeviceListCtx& ctx) {
            qCDebug(lcDeviceController) << "DeviceListCtx" << ctx.res.status;

            auto finishTask = [this]() {
                ra_finished.store(true);
                check_finished();
            };

            if (ctx.res.status != 200) {
                if (requiresRemoteAccessPrompt(ctx.res.status)) {
                    initAccessCode();
                } else {
                    qCWarning(lcDeviceController) << "Device list request failed, finishing account update"
                                                  << ctx.res.status << ctx.res.errorString;
                    finishTask();
                }
                return;
            }

            const auto dev_ra = ctx.deviceList.find_by_cn(d.certificateCommonName);
            if (!dev_ra || dev_ra->seagateDeviceID.isEmpty()) {
                logRemoteAccessCnLookupMismatch("account_update_device:", d, ctx.deviceList);
                qCDebug(lcDeviceController) << "Device not found or ID empty (CN lookup)";
                finishTask();
                return;
            }

            applyRemoteAccessIdentity(currentDevice, *dev_ra);
            logRemoteAccessIdentityApplied("account_update_device:", currentDevice);
            qCDebug(lcDeviceController) << "Query device info for" << dev_ra->seagateDeviceID;
            queryDeviceInfo(dev_ra->seagateDeviceID)
                .then(this, [this, finishTask, id=dev_ra->seagateDeviceID](const DevicePathListCtx& ctx) {
                    qCDebug(lcDeviceController) << "acc update ra_device_info code" << ctx.res.status << ctx.res.errorString;

                    if (ctx.res.status == 200) {
                        currentDevice.updateRemotePathCache(ctx.devicePathList);
                        _devApi->query_about_all(currentDevice.remotePaths())
                            .then(this, [this,finishTask](const QList<DevicePath>& paths) {
                                qCDebug(lcDeviceController) << "About updated:" << paths;

                                currentDevice.updateRemotePathCache(paths);
                                logRemoteAccessIdentityApplied("account_update_device paths_updated:", currentDevice);
                                replaceAccountRemotePaths(allAccountPaths, currentDevice.remotePaths());
                                finishTask();
                            })
                            .onFailed(this, [finishTask](const std::exception& e) {
                                qCWarning(lcDeviceController) << "Remote path about request exception, finishing account update" << e.what();
                                finishTask();
                            });
                    }
                    else if (ctx.res.status == 404) {
                        // Device not found
                        qCDebug(lcDeviceController) << "Device not found" << id;
                        finishTask();
                    }
                    else if (requiresRemoteAccessPrompt(ctx.res.status)) {
                        initAccessCode();
                    } else {
                        qCWarning(lcDeviceController) << "Device info request failed, finishing account update"
                                                      << ctx.res.status << ctx.res.errorString;
                        finishTask();
                    }
                }).onFailed(this, [finishTask](const std::exception& e) {
                    qCWarning(lcDeviceController) << "Device info request exception, finishing account update" << e.what();
                    finishTask();
                });
    }).onFailed(this, [this](const std::exception& e) {
        qCWarning(lcDeviceController) << "Device list request exception, finishing account update" << e.what();
        ra_finished.store(true);
        check_finished();
    });
}

void DeviceController::finishAccountUpdateWithDiscoveredPaths(const QString& devCN)
{
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
            })
            .onFailed(this, [this, devCN](const std::exception& e) {
                qCWarning(lcDeviceController) << "Path status request exception for" << devCN << e.what();
                emit account_update_device_finished(currentDevice);
            });
    }
    else {
        emit account_update_device_finished(currentDevice);
    }
}

void DeviceController::account_update_device_continue(std::optional<Device> dev)
{
    if (!dev) {
        qCDebug(lcDeviceController) << "acc update continue: no account device";
        ra_finished.store(true);
        check_finished();
        return;
    }

    currentDevice = *dev;
    auto finishTask = [this]() {
        ra_finished.store(true);
        check_finished();
    };

    auto continueWithResolvedDevice = [this, finishTask](const Device& resolvedDevice) {
        currentDevice = resolvedDevice;

        queryDeviceInfo(resolvedDevice.seagateDeviceID)
            .then(this, [this, finishTask](const DevicePathListCtx& ctx) {
                qCDebug(lcDeviceController) << "acc update continue ra_device_info code" << ctx.res.status << ctx.res.errorString;
                if (ctx.res.status == 200) {
                    currentDevice.updateRemotePathCache(ctx.devicePathList);

                    _devApi->query_about_all(currentDevice.remotePaths())
                        .then(this, [this, finishTask](const QList<DevicePath>& paths) {
                            qCDebug(lcDeviceController) << "Paths added:";
                            qCDebug(lcDeviceController) << paths;
                            currentDevice.updateRemotePathCache(paths);
                            logRemoteAccessIdentityApplied("account_update_device_continue paths_updated:", currentDevice);
                            replaceAccountRemotePaths(allAccountPaths, currentDevice.remotePaths());
                            finishTask();
                        })
                        .onFailed(this, [finishTask](const std::exception& e) {
                            qCWarning(lcDeviceController) << "Remote path about request exception, finishing account update" << e.what();
                            finishTask();
                        });

                }
                else {
                    qCDebug(lcDeviceController) << "acc update continue ra_device_info fail";
                    finishTask();
                }
            })
            .onFailed(this, [finishTask](const std::exception& e) {
                qCWarning(lcDeviceController) << "Device info request exception, finishing account update" << e.what();
                finishTask();
            });
    };

    if (!currentDevice.seagateDeviceID.isEmpty()) {
        continueWithResolvedDevice(currentDevice);
        return;
    }

    qCDebug(lcDeviceController) << "acc update continue: resolving device identity from RA list";
    queryDeviceList()
        .then(this, [this, continueWithResolvedDevice, finishTask](const DeviceListCtx& ctx) {
            qCDebug(lcDeviceController) << "acc update continue device list code" << ctx.res.status << ctx.res.errorString;
            if (ctx.res.status != 200) {
                finishTask();
                return;
            }

            const auto resolvedDevice = ctx.deviceList.find_by_cn(currentDevice.certificateCommonName);
            if (!resolvedDevice || resolvedDevice->seagateDeviceID.isEmpty()) {
                logRemoteAccessCnLookupMismatch("account_update_device_continue:", currentDevice, ctx.deviceList);
                qCDebug(lcDeviceController) << "acc update continue: device not found or ID empty (CN lookup)";
                finishTask();
                return;
            }

            Device updatedDevice = currentDevice;
            applyRemoteAccessIdentity(updatedDevice, *resolvedDevice);
            logRemoteAccessIdentityApplied("account_update_device_continue:", updatedDevice);
            continueWithResolvedDevice(updatedDevice);
        })
        .onFailed(this, [finishTask](const std::exception& e) {
            qCWarning(lcDeviceController) << "Device list request exception, finishing account update" << e.what();
            finishTask();
        });
}


void DeviceController::force_ra_account()
{
    mdns_finished.store(false);
    ra_finished.store(false);

    _mdnsDeviceList.clear();
    _raDeviceList.clear();
    _aggregator->clearAll();
    allAccountPaths.clear();
    force_device_list_request = true;
    _deferredRaQuery = DeferredRaQuery::ForceAccount;
    qCDebug(lcDeviceController) << "Forced RA discovery deferred until mDNS completes";
    startMdnsDiscovery();
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
    const auto email = normalizedEmail(_email);
    const auto token = _api->tokenCtx().refreshToken;
    ++_refreshTokenGeneration;
    _refreshTokenLoadedEmail = email;
    saveRefreshTokenToSecureStorage(email, token);
    qCDebug(lcDeviceController) << "Refresh token save to secure storage, token exist" << !token.isEmpty();
}

QFuture<void> DeviceController::loadRefreshToken()
{
    const auto email = normalizedEmail(_email);
    const auto generation = _refreshTokenGeneration;
    if (email == _refreshTokenLoadedEmail && _api->hasRefreshToken()) {
        return makeReadyVoidFuture();
    }

    APP::ConfigFile cf;
    const auto legacyToken = cf.refreshTokenForEmail(email);
    if (!legacyToken.isEmpty()) {
        _api->setRefreshToken(legacyToken);
    }

    if (email.isEmpty()) {
        qCDebug(lcDeviceController) << "Refresh token load skipped, empty email";
        return makeReadyVoidFuture();
    }

    const auto key = refreshTokenCredentialKey(email);
    if (!_credentialManager->contains(key)) {
        if (!legacyToken.isEmpty()) {
            saveRefreshTokenToSecureStorage(email, legacyToken);
        }
        _refreshTokenLoadedEmail = email;
        qCDebug(lcDeviceController) << "Refresh token load from legacy settings, token exist" << !legacyToken.isEmpty();
        return makeReadyVoidFuture();
    }

    auto promise = std::make_shared<QPromise<void>>();
    auto future = promise->future();
    auto job = _credentialManager->get(key);
    connect(job, &APP::CredentialJob::finished, this, [this, job, promise, email, legacyToken, generation] {
        if (generation != _refreshTokenGeneration || email != normalizedEmail(_email)) {
            promise->finish();
            return;
        }

        if (job->error() == QKeychain::NoError) {
            const auto token = job->data().toString();
            _api->setRefreshToken(token);
            if (!legacyToken.isEmpty()) {
                APP::ConfigFile cf;
                cf.removeRefreshTokenForEmail(email);
            }
            _refreshTokenLoadedEmail = email;
            qCDebug(lcDeviceController) << "Refresh token load from secure storage, token exist" << !token.isEmpty();
        } else {
            if (!legacyToken.isEmpty()) {
                saveRefreshTokenToSecureStorage(email, legacyToken);
            }
            _refreshTokenLoadedEmail = email;
            qCWarning(lcDeviceController) << "Refresh token secure storage load failed, using legacy fallback"
                                          << job->errorString();
        }

        promise->finish();
    });
    return future;
}

void DeviceController::saveRefreshTokenToSecureStorage(const QString& email, const QString& token)
{
    if (email.isEmpty() || token.isEmpty()) {
        return;
    }

    auto job = _credentialManager->set(refreshTokenCredentialKey(email), token);
    connect(job, &QKeychain::Job::finished, this, [job, email] {
        if (job->error() == QKeychain::NoError) {
            APP::ConfigFile cf;
            cf.removeRefreshTokenForEmail(email);
        }
    });
}

void DeviceController::removeRefreshTokenFromSecureStorage(const QString& email)
{
    const auto normalized = normalizedEmail(email);
    if (normalized.isEmpty()) {
        return;
    }

    APP::ConfigFile cf;
    cf.removeRefreshTokenForEmail(normalized);

    const auto key = refreshTokenCredentialKey(normalized);
    if (_credentialManager->contains(key)) {
        _credentialManager->remove(key);
    }

    if (normalized == normalizedEmail(_email)) {
        ++_refreshTokenGeneration;
        _refreshTokenLoadedEmail.clear();
        _api->clearTokens();
    }

    qCWarning(lcDeviceController) << "Removed invalid Remote Access refresh token for email" << normalized;
}

DeviceList DeviceController::getDevices() const
{
    QReadLocker locker(&lock_);

    return DeviceAggregator::mergeDevices(_mdnsDeviceList, _raDeviceList);
}

QFuture<DeviceListCtx> DeviceController::queryDeviceList()
{
    const auto requestEmail = normalizedEmail(_email);
    return loadRefreshToken()
        .then(this, [this, requestEmail] {
            return _api->ra_device_list()
                .then(this, [this, requestEmail](const DeviceListCtx& ctx) {
                    if (requestEmail != normalizedEmail(_email)) {
                        qCDebug(lcDeviceController) << "Ignoring stale Remote Access device list token persistence for email" << requestEmail;
                        return ctx;
                    }

                    if (ctx.res.status == 200) {
                        saveRefreshToken();
                    } else if (isInvalidRefreshTokenError(ctx.res)) {
                        removeRefreshTokenFromSecureStorage(requestEmail);
                    }
                    return ctx;
                });
        })
        .unwrap();
}

QFuture<DevicePathListCtx> DeviceController::queryDeviceInfo(const QString &deviceId)
{
    const auto requestEmail = normalizedEmail(_email);
    return loadRefreshToken()
        .then(this, [this, deviceId, requestEmail] {
            return _api->ra_device_info(deviceId)
                .then(this, [this, requestEmail](const DevicePathListCtx& ctx) {
                    if (requestEmail != normalizedEmail(_email)) {
                        qCDebug(lcDeviceController) << "Ignoring stale Remote Access device info token persistence for email" << requestEmail;
                        return ctx;
                    }

                    if (ctx.res.status == 200) {
                        saveRefreshToken();
                    } else if (isInvalidRefreshTokenError(ctx.res)) {
                        removeRefreshTokenFromSecureStorage(requestEmail);
                    }
                    return ctx;
                });
        })
        .unwrap();
}

QFuture<DevicePathResolutionResult> DeviceController::resolveDevicePath(const Device& device, const std::optional<QUuid>& avoidPathId,
    const std::optional<QUuid>& preferredPathId)
{
    if (!_pathResolver) {
        DevicePathResolutionResult result;
        result.device = device;
        return QtFuture::makeReadyValueFuture(result);
    }

    return _pathResolver->resolve(device, avoidPathId, preferredPathId);
}

void DeviceController::initAccessCode()
{
    qCDebug(lcDeviceController) << "initAccessCode";
    ++_refreshTokenGeneration;
    _api->clearTokens();
    _refreshTokenLoadedEmail.clear();
    _api->ra_initiate(_email)
        .then(this, [this](const InitContext& ctx) {
            qCWarning(lcDeviceController) << "initAccessCode result" << ctx.res.status << ctx.res.errorString;
            if (ctx.res.status == 200) {
                emit accessCodeRequest();
            }
            else {
                emit accessCodeResult(AccessCodeContext::Init, ctx.res.status, ctx.res.errorString, ctx.res.errorStacktrace);
            }
        })
        .onFailed(this, [this](const std::exception& e) {
            qCWarning(lcDeviceController) << "initAccessCode exception" << e.what();
            emit accessCodeResult(AccessCodeContext::Init, 0, QString::fromUtf8(e.what()), {});
        });
}

void DeviceController::enterAccessCode(const QString &code, bool from_account)
{
    qCDebug(lcDeviceController) << "enterAccessCode, from account:" << from_account;
    _api->ra_token(code)
        .then(this, [this](const TokenContext& ctx) {
            qCDebug(lcDeviceController) << "enterAccessCode" << ctx.res.status << ctx.res.errorString;
            if (ctx.res.status == 200) {
                saveRefreshToken();
                emit accessCodeResult(AccessCodeContext::Token, ctx.res.status, {}, {});
            }
            else {
                emit accessCodeResult(AccessCodeContext::Token, ctx.res.status, ctx.res.errorString, ctx.res.errorStacktrace);
            }
        })
        .onFailed(this, [this](const std::exception& e) {
            qCWarning(lcDeviceController) << "enterAccessCode exception" << e.what();
            emit accessCodeResult(AccessCodeContext::Token, 0, QString::fromUtf8(e.what()), {});
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
                    qCInfo(lcDeviceController) << "RA discovery device list merged"
                                               << "rawDeviceCount" << ctx.deviceList.devices().size()
                                               << "mergedRaDeviceCount" << _raDeviceList.devices().size();
                }
                qCDebug(lcDeviceController) << "Device list:";
                qCDebug(lcDeviceController) << ctx.deviceList;
                saveRefreshToken();
            }
            ra_finished.store(true);
            check_finished();
        })
        .onFailed(this, [this](const std::exception& e) {
            qCWarning(lcDeviceController) << "Device list request exception, finishing RA discovery" << e.what();
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
                    qCInfo(lcDeviceController) << "Forced RA discovery device list merged"
                                               << "rawDeviceCount" << ctx.deviceList.devices().size()
                                               << "mergedRaDeviceCount" << _raDeviceList.devices().size();
                    // _deviceList = ctx.deviceList;
                }

                qCDebug(lcDeviceController) << "Device list:";
                qCDebug(lcDeviceController) << ctx.deviceList;
                saveRefreshToken();
                ra_finished.store(true);
                check_finished();
            }
            else {
                if (requiresRemoteAccessPrompt(ctx.res.status)) {
                    initAccessCode();
                } else {
                    qCWarning(lcDeviceController) << "Forced device list request failed, finishing RA discovery"
                                                  << ctx.res.status << ctx.res.errorString;
                    ra_finished.store(true);
                    check_finished();
                }
            }
        })
        .onFailed(this, [this](const std::exception& e) {
            qCWarning(lcDeviceController) << "Forced device list request exception, finishing RA discovery" << e.what();
            ra_finished.store(true);
            check_finished();
        });
}

void DeviceController::startMdnsDiscovery()
{
    ++_mdnsDiscoveryGeneration;
    qCDebug(lcDeviceController) << "Starting mDNS discovery generation" << _mdnsDiscoveryGeneration;
    _mdns->start();
}

void DeviceController::finishMdnsDiscoveryPhase(quint64 generation)
{
    if (generation != _mdnsDiscoveryGeneration) {
        qCDebug(lcDeviceController) << "Ignoring stale mDNS discovery result"
                                    << "resultGeneration" << generation
                                    << "currentGeneration" << _mdnsDiscoveryGeneration;
        return;
    }

    mdns_finished.store(true);

    const auto deferredRaQuery = _deferredRaQuery;
    _deferredRaQuery = DeferredRaQuery::None;

    switch (deferredRaQuery) {
    case DeferredRaQuery::NewAccount:
        qCDebug(lcDeviceController) << "mDNS completed, starting deferred RA device list request";
        processQueryDeviceList();
        return;
    case DeferredRaQuery::ForceAccount:
        qCDebug(lcDeviceController) << "mDNS completed, starting deferred forced RA device list request";
        forceQueryDeviceList();
        return;
    case DeferredRaQuery::AccountUpdate:
        qCDebug(lcDeviceController) << "mDNS completed, starting deferred RA account update request";
        processAccountUpdateDeviceList();
        return;
    case DeferredRaQuery::LocalAccountUpdate:
        qCDebug(lcDeviceController) << "mDNS completed, finishing local account path update";
        finishAccountUpdateWithDiscoveredPaths(currentDevice.certificateCommonName);
        return;
    case DeferredRaQuery::None:
        check_finished();
        return;
    }
}

void DeviceController::check_finished()
{
    bool m = mdns_finished.load();
    bool r = ra_finished.load();

    qCDebug(lcDeviceController) << "mdns finished" << m << "ra finished" << r;
    if (m && r) {
        int mdnsDeviceCount = 0;
        int raDeviceCount = 0;
        int mergedDeviceCount = 0;
        {
            QReadLocker locker(&lock_);
            mdnsDeviceCount = _mdnsDeviceList.devices().size();
            raDeviceCount = _raDeviceList.devices().size();
            mergedDeviceCount = DeviceAggregator::mergeDevices(_mdnsDeviceList, _raDeviceList).devices().size();
        }
        qCInfo(lcDeviceController) << "Device discovery completed"
                                   << "mdnsDeviceCount" << mdnsDeviceCount
                                   << "raDeviceCount" << raDeviceCount
                                   << "mergedDeviceCount" << mergedDeviceCount
                                   << "raAvailable" << hasRefreshToken();
        emit devices_updated(hasRefreshToken());
        force_device_list_request = false;
        mdns_finished.store(false);
        ra_finished.store(false);
    }
}
