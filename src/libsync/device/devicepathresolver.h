#pragma once

#include "personalcloudlib.h"
#include "deviceapi.h"
#include "apiclient.h"

#include <QSet>

#include <functional>
#include <optional>

struct APPLICATIONSYNC_EXPORT DevicePathResolutionResult
{
    Device device;
    std::optional<QUuid> selectedPathId;
    ResultContext remoteAccessResult;
    bool remoteAccessRequested = false;
    bool remoteCacheUpdated = false;
    bool remoteCacheTimestampRefreshed = false;
    bool usedRemoteRelay = false;

    bool resolved() const { return selectedPathId.has_value(); }
};

class APPLICATIONSYNC_EXPORT DevicePathResolver : public QObject
{
    Q_OBJECT

public:
    using QueryDeviceInfoFn = std::function<QFuture<DevicePathListCtx>(const QString&)>;

    explicit DevicePathResolver(DeviceApi* deviceApi, QueryDeviceInfoFn queryDeviceInfo, QObject* parent = nullptr);

    QFuture<DevicePathResolutionResult> resolve(const Device& device);

private:
    QFuture<DevicePathResolutionResult> testPriorityPaths(Device device, const QList<DevicePath>& paths, const DevicePathResolutionResult& result);
    QFuture<DevicePathResolutionResult> resolveAfterPriorityFailure(Device device, const QSet<QString>& testedPriorityKeys, const DevicePathResolutionResult& result);
    QFuture<DevicePathResolutionResult> testRelayPath(Device device, const DevicePathResolutionResult& result);

    DeviceApi* _deviceApi = nullptr;
    QueryDeviceInfoFn _queryDeviceInfo;
};

Q_DECLARE_METATYPE(DevicePathResolutionResult);
