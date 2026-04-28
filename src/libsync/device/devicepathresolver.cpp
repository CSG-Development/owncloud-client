#include "devicepathresolver.h"

#include <QtConcurrent>

#include <algorithm>
#include <QLoggingCategory>

namespace {

Q_LOGGING_CATEGORY(lcDevicePathResolver, "device.pathresolver", QtDebugMsg)

QString pathKey(const DevicePath& path)
{
    return QStringLiteral("%1|%2|%3")
        .arg(path.address, QString::number(path.port), DevHelpers::devTypeToStr(path.deviceType));
}

QSet<QString> pathKeys(const QList<DevicePath>& paths)
{
    QSet<QString> keys;
    for (const auto& path : paths) {
        keys.insert(pathKey(path));
    }
    return keys;
}

bool isPriorityPath(const DevicePath& path)
{
    return path.deviceType == DeviceType::Local || path.deviceType == DeviceType::Public;
}

bool isRelayPath(const DevicePath& path)
{
    return path.deviceType == DeviceType::Remote;
}

QList<DevicePath> priorityPaths(const Device& device)
{
    QList<DevicePath> paths;
    for (const auto& path : device.paths) {
        if (isPriorityPath(path)) {
            paths.append(path);
        }
    }

    std::stable_sort(paths.begin(), paths.end(), [](const DevicePath& a, const DevicePath& b) {
        return DevicePath::pathPriority(a.deviceType) > DevicePath::pathPriority(b.deviceType);
    });

    return paths;
}

QList<DevicePath> relayPaths(const Device& device)
{
    QList<DevicePath> paths;
    for (const auto& path : device.paths) {
        if (isRelayPath(path)) {
            paths.append(path);
        }
    }
    return paths;
}

QList<DevicePath> excludeTestedPaths(const QList<DevicePath>& paths, const QSet<QString>& testedPathKeys)
{
    QList<DevicePath> filtered;
    for (const auto& path : paths) {
        if (!testedPathKeys.contains(pathKey(path))) {
            filtered.append(path);
        }
    }
    return filtered;
}

QList<DevicePath> excludePathId(const QList<DevicePath>& paths, const std::optional<QUuid>& avoidPathId, QSet<QString>* skippedPathKeys)
{
    if (!avoidPathId || avoidPathId->isNull()) {
        return paths;
    }

    QList<DevicePath> filtered;
    for (const auto& path : paths) {
        if (path.id == avoidPathId.value()) {
            if (skippedPathKeys) {
                skippedPathKeys->insert(pathKey(path));
            }
            continue;
        }

        filtered.append(path);
    }
    return filtered;
}

void mergeUpdatedPaths(Device& device, const QList<DevicePath>& updatedPaths)
{
    for (const auto& updatedPath : updatedPaths) {
        if (auto* existingPath = device.getPathPtr(updatedPath.id)) {
            *existingPath = updatedPath;
            continue;
        }

        const auto it = std::find_if(device.paths.begin(), device.paths.end(), [&updatedPath](const DevicePath& currentPath) {
            return currentPath.address == updatedPath.address
                && currentPath.port == updatedPath.port
                && currentPath.deviceType == updatedPath.deviceType;
        });

        if (it != device.paths.end()) {
            *it = updatedPath;
        } else {
            device.paths.append(updatedPath);
        }
    }
}

DevicePathResolutionResult withDevice(const DevicePathResolutionResult& result, const Device& device)
{
    auto updatedResult = result;
    updatedResult.device = device;
    return updatedResult;
}

}

DevicePathResolver::DevicePathResolver(DeviceApi* deviceApi, QueryDeviceInfoFn queryDeviceInfo, QObject* parent)
    : QObject(parent)
    , _deviceApi(deviceApi)
    , _queryDeviceInfo(std::move(queryDeviceInfo))
{
}

QFuture<DevicePathResolutionResult> DevicePathResolver::resolve(const Device& sourceDevice, const std::optional<QUuid>& avoidPathId)
{
    Device device = sourceDevice;
    DevicePathResolutionResult result;
    result.device = device;
    qCDebug(lcDevicePathResolver) << "Starting resolution for" << device.toStringShort();

    QSet<QString> skippedPriorityKeys;
    const auto cachedPriorityPaths = excludePathId(priorityPaths(device), avoidPathId, &skippedPriorityKeys);
    if (cachedPriorityPaths.isEmpty() || !_deviceApi) {
        qCDebug(lcDevicePathResolver) << "No cached priority paths available, going to RA/relay fallback";
        return resolveAfterPriorityFailure(device, skippedPriorityKeys, result);
    }

    if (!skippedPriorityKeys.isEmpty()) {
        qCDebug(lcDevicePathResolver) << "Testing cached priority paths except avoided path" << cachedPriorityPaths;
    } else {
        qCDebug(lcDevicePathResolver) << "Testing cached priority paths" << cachedPriorityPaths;
    }
    return testPriorityPaths(device, cachedPriorityPaths, result, DevicePathResolutionOutcome::ResolvedFromCachedPriority)
        .then(this, [this, cachedPriorityPaths, skippedPriorityKeys](const DevicePathResolutionResult& testResult) {
            if (testResult.resolved()) {
                qCDebug(lcDevicePathResolver) << "Resolved from cached priority path";
                return QtFuture::makeReadyValueFuture(testResult);
            }

            qCDebug(lcDevicePathResolver) << "Cached priority paths failed, forcing fresh RA refresh";
            auto testedPriorityKeys = pathKeys(cachedPriorityPaths);
            testedPriorityKeys.unite(skippedPriorityKeys);
            return resolveAfterPriorityFailure(testResult.device, testedPriorityKeys, testResult);
        })
        .unwrap();
}

QFuture<DevicePathResolutionResult> DevicePathResolver::testPriorityPaths(Device device, const QList<DevicePath>& paths, const DevicePathResolutionResult& result,
    DevicePathResolutionOutcome successOutcome)
{
    if (!_deviceApi || paths.isEmpty()) {
        return QtFuture::makeReadyValueFuture(withDevice(result, device));
    }

    struct PriorityTestState {
        Device device;
        DevicePathResolutionResult result;
        int pendingCount = 0;
        int localPendingCount = 0;
        bool finished = false;
        std::optional<DevicePath> firstSuccessfulPublicPath;
        std::shared_ptr<QPromise<DevicePathResolutionResult>> promise;
    };

    auto state = std::make_shared<PriorityTestState>();
    state->device = device;
    state->result = result;
    state->result.device = device;
    state->pendingCount = paths.size();
    state->localPendingCount = std::count_if(paths.cbegin(), paths.cend(), [](const DevicePath& path) {
        return path.deviceType == DeviceType::Local;
    });
    state->promise = std::make_shared<QPromise<DevicePathResolutionResult>>();

    auto finishWithCurrentDevice = [state]() mutable {
        if (state->finished) {
            return;
        }

        state->finished = true;
        state->result.device = state->device;
        state->promise->addResult(state->result);
        state->promise->finish();
    };

    for (const auto& path : paths) {
        const auto url = DevHelpers::makeServerUrl(path.address, path.port, false, true, path.origin);
        _deviceApi->query_status(url)
            .then(this, [state, path, finishWithCurrentDevice, successOutcome](const StatusCtx& ctx) mutable {
                if (state->finished) {
                    return;
                }

                DevicePath updatedPath = path;
                updatedPath.status = ctx.deviceStatus;
                mergeUpdatedPaths(state->device, { updatedPath });

                --state->pendingCount;
                if (path.deviceType == DeviceType::Local) {
                    --state->localPendingCount;
                }

                if (ctx.status == 200 && updatedPath.status.oobe_done) {
                    if (path.deviceType == DeviceType::Local) {
                        state->result.selectedPathId = updatedPath.id;
                        state->result.outcome = successOutcome;
                        finishWithCurrentDevice();
                        return;
                    }

                    if (!state->firstSuccessfulPublicPath) {
                        state->firstSuccessfulPublicPath = updatedPath;
                    }
                }

                if (state->localPendingCount == 0 && state->firstSuccessfulPublicPath) {
                    state->result.selectedPathId = state->firstSuccessfulPublicPath->id;
                    state->result.outcome = successOutcome;
                    finishWithCurrentDevice();
                    return;
                }

                if (state->pendingCount == 0) {
                    finishWithCurrentDevice();
                }
            });
    }

    return state->promise->future();
}

QFuture<DevicePathResolutionResult> DevicePathResolver::resolveAfterPriorityFailure(Device device, const QSet<QString>& testedPriorityKeys, const DevicePathResolutionResult& result)
{
    const auto canQueryRemotePaths = static_cast<bool>(_queryDeviceInfo) && !device.seagateDeviceID.isEmpty();
    if (!canQueryRemotePaths) {
        qCDebug(lcDevicePathResolver) << "Fresh RA refresh is unavailable, trying relay paths only";
        return testRelayPath(device, result)
            .then(this, [device, canRequestPrompt = static_cast<bool>(_queryDeviceInfo)](const DevicePathResolutionResult& relayResult) {
                if (relayResult.resolved()) {
                    return QtFuture::makeReadyValueFuture(relayResult);
                }

                auto updatedResult = relayResult;
                if (canRequestPrompt && !device.certificateCommonName.isEmpty()) {
                    updatedResult.outcome = DevicePathResolutionOutcome::RequiresRemoteAccessPrompt;
                }
                return QtFuture::makeReadyValueFuture(updatedResult);
            })
            .unwrap();
    }

    return _queryDeviceInfo(device.seagateDeviceID)
        .then(this, [this, device, testedPriorityKeys, result](const DevicePathListCtx& ctx) mutable {
            auto updatedResult = withDevice(result, device);
            updatedResult.remoteAccessRequested = true;
            updatedResult.remoteAccessResult = ctx.res;
            qCDebug(lcDevicePathResolver) << "Fresh RA refresh finished with status" << ctx.res.status;

            if (ctx.res.status != 200) {
                if (!device.certificateCommonName.isEmpty()
                    && (ctx.res.status == 401 || ctx.res.status == 403 || ctx.res.status == -2)) {
                    updatedResult.outcome = DevicePathResolutionOutcome::RequiresRemoteAccessPrompt;
                }
                return testRelayPath(device, updatedResult);
            }

            const auto fetchedAtUtc = QDateTime::currentDateTimeUtc();
            const auto normalizedRemotePaths = Device::normalizeRemotePaths(ctx.devicePathList);

            if (device.hasRemotePathCache() && device.hasSameRemotePaths(normalizedRemotePaths)) {
                device.remotePathsFetchedAtUtc = fetchedAtUtc;
                updatedResult.device = device;
                updatedResult.remoteCacheTimestampRefreshed = true;
                qCDebug(lcDevicePathResolver) << "Fresh RA refresh returned the same remote path set";
                return testRelayPath(device, updatedResult);
            }

            device.updateRemotePathCache(normalizedRemotePaths, fetchedAtUtc);
            updatedResult.device = device;
            updatedResult.remoteCacheUpdated = true;
            qCDebug(lcDevicePathResolver) << "Fresh RA refresh updated remote path cache";

            if (!_deviceApi) {
                return testRelayPath(device, updatedResult);
            }

            const auto freshPriorityPaths = excludeTestedPaths(priorityPaths(device), testedPriorityKeys);
            if (freshPriorityPaths.isEmpty()) {
                return testRelayPath(device, updatedResult);
            }

            qCDebug(lcDevicePathResolver) << "Testing fresh priority paths" << freshPriorityPaths;
            return testPriorityPaths(device, freshPriorityPaths, updatedResult, DevicePathResolutionOutcome::ResolvedFromFreshRemoteAccess)
                .then(this, [this](const DevicePathResolutionResult& freshPriorityResult) {
                    if (freshPriorityResult.resolved()) {
                        qCDebug(lcDevicePathResolver) << "Resolved from fresh RA priority path";
                        return QtFuture::makeReadyValueFuture(freshPriorityResult);
                    }

                    qCDebug(lcDevicePathResolver) << "Fresh priority paths failed, falling back to relay paths";
                    return testRelayPath(freshPriorityResult.device, freshPriorityResult);
                })
                .unwrap();
        })
        .unwrap();
}

QFuture<DevicePathResolutionResult> DevicePathResolver::testRelayPath(Device device, const DevicePathResolutionResult& result)
{
    if (!_deviceApi) {
        return QtFuture::makeReadyValueFuture(result);
    }

    const auto remoteRelayPaths = relayPaths(device);
    if (remoteRelayPaths.isEmpty()) {
        return QtFuture::makeReadyValueFuture(result);
    }

    qCDebug(lcDevicePathResolver) << "Testing relay paths" << remoteRelayPaths;
    return _deviceApi->query_status_all(remoteRelayPaths)
        .then(this, [device, result](const QList<DevicePath>& updatedPaths) mutable {
            mergeUpdatedPaths(device, updatedPaths);

            auto updatedResult = withDevice(result, device);
            const auto successfulRelayPath = std::find_if(updatedPaths.cbegin(), updatedPaths.cend(), [](const DevicePath& path) {
                return path.status.oobe_done;
            });
            if (successfulRelayPath != updatedPaths.cend()) {
                updatedResult.selectedPathId = successfulRelayPath->id;
                updatedResult.outcome = DevicePathResolutionOutcome::ResolvedFromRemoteRelay;
                updatedResult.usedRemoteRelay = true;
            } else {
                qCDebug(lcDevicePathResolver) << "Resolution exhausted without reachable paths";
            }

            return updatedResult;
        });
}
