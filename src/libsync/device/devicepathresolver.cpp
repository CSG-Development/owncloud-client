#include "devicepathresolver.h"

#include <QtConcurrent>

#include <algorithm>

namespace {

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

QFuture<DevicePathResolutionResult> DevicePathResolver::resolve(const Device& sourceDevice)
{
    Device device = sourceDevice;
    DevicePathResolutionResult result;
    result.device = device;

    const auto cachedPriorityPaths = priorityPaths(device);
    if (cachedPriorityPaths.isEmpty() || !_deviceApi) {
        return resolveAfterPriorityFailure(device, {}, result);
    }

    return testPriorityPaths(device, cachedPriorityPaths, result)
        .then(this, [this, cachedPriorityPaths](const DevicePathResolutionResult& testResult) {
            if (testResult.resolved()) {
                return QtFuture::makeReadyValueFuture(testResult);
            }

            return resolveAfterPriorityFailure(testResult.device, pathKeys(cachedPriorityPaths), testResult);
        })
        .unwrap();
}

QFuture<DevicePathResolutionResult> DevicePathResolver::testPriorityPaths(Device device, const QList<DevicePath>& paths, const DevicePathResolutionResult& result)
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
        const auto url = DevHelpers::makeServerUrl(path.address, path.port, false, true);
        _deviceApi->query_status(url)
            .then(this, [state, path, finishWithCurrentDevice](const StatusCtx& ctx) mutable {
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
                        finishWithCurrentDevice();
                        return;
                    }

                    if (!state->firstSuccessfulPublicPath) {
                        state->firstSuccessfulPublicPath = updatedPath;
                    }
                }

                if (state->localPendingCount == 0 && state->firstSuccessfulPublicPath) {
                    state->result.selectedPathId = state->firstSuccessfulPublicPath->id;
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
    const auto shouldFetchRemotePaths = canQueryRemotePaths && (!device.hasRemotePathCache() || device.isRemotePathCacheExpired());

    if (!shouldFetchRemotePaths) {
        return testRelayPath(device, result);
    }

    return _queryDeviceInfo(device.seagateDeviceID)
        .then(this, [this, device, testedPriorityKeys, result](const DevicePathListCtx& ctx) mutable {
            auto updatedResult = withDevice(result, device);
            updatedResult.remoteAccessRequested = true;
            updatedResult.remoteAccessResult = ctx.res;

            if (ctx.res.status != 200) {
                return testRelayPath(device, updatedResult);
            }

            const auto fetchedAtUtc = QDateTime::currentDateTimeUtc();
            const auto normalizedRemotePaths = Device::normalizeRemotePaths(ctx.devicePathList);

            if (device.hasRemotePathCache() && device.hasSameRemotePaths(normalizedRemotePaths)) {
                device.remotePathsFetchedAtUtc = fetchedAtUtc;
                updatedResult.device = device;
                updatedResult.remoteCacheTimestampRefreshed = true;
                return testRelayPath(device, updatedResult);
            }

            device.updateRemotePathCache(normalizedRemotePaths, fetchedAtUtc);
            updatedResult.device = device;
            updatedResult.remoteCacheUpdated = true;

            if (!_deviceApi) {
                return testRelayPath(device, updatedResult);
            }

            const auto freshPriorityPaths = excludeTestedPaths(priorityPaths(device), testedPriorityKeys);
            if (freshPriorityPaths.isEmpty()) {
                return testRelayPath(device, updatedResult);
            }

            return testPriorityPaths(device, freshPriorityPaths, updatedResult)
                .then(this, [this](const DevicePathResolutionResult& freshPriorityResult) {
                    if (freshPriorityResult.resolved()) {
                        return QtFuture::makeReadyValueFuture(freshPriorityResult);
                    }

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

    return _deviceApi->query_status_all(remoteRelayPaths)
        .then(this, [device, result](const QList<DevicePath>& updatedPaths) mutable {
            mergeUpdatedPaths(device, updatedPaths);

            auto updatedResult = withDevice(result, device);
            const auto successfulRelayPath = std::find_if(updatedPaths.cbegin(), updatedPaths.cend(), [](const DevicePath& path) {
                return path.status.oobe_done;
            });
            if (successfulRelayPath != updatedPaths.cend()) {
                updatedResult.selectedPathId = successfulRelayPath->id;
                updatedResult.usedRemoteRelay = true;
            }

            return updatedResult;
        });
}
