#pragma once

#include "curatorlib.h"
#include "devicedefines.h"

#include <QString>
#include <QList>
#include <QDateTime>
#include <QUuid>

class CURATORSYNC_EXPORT DevicePath
{
public:
    DevicePath();
    DevicePath(const QString& addr, DeviceType devType, DeviceOrigin org, int pport);

    QString address;
    DeviceType deviceType = DeviceType::Unknown;
    DeviceOrigin origin = DeviceOrigin::Unknown;    // Mostly for debug
    int port = 0;

    QUuid id;                   // Not saved in config, runtime only
    DeviceInfoAbout about;      // Not saved in config, runtime only
    DeviceInfoStatus status;    // Not saved in config, runtime only
    QString friendlyName;       // Not saved in config, runtime only

    QJsonObject toJson() const;
    static DevicePath fromJson(const QJsonObject &val);
    static int pathPriority(DeviceType devType);

    QString toString() const;
    QString toStringShort() const;
};

class CURATORSYNC_EXPORT Device
{
public:
    QString seagateDeviceID;
    QString certificateCommonName;
    QString friendlyName;
    QString hostname;
    bool isStatic = false;
    DeviceOrigin origin = DeviceOrigin::Unknown;
    QList<DevicePath> paths;

    static Device MakeStatic(const QString& url, const QString& name);

    static QJsonDocument toJson(const Device& dev);
    static QByteArray toJsonStr(const Device& dev);

    static Device fromJson(const QJsonDocument &obj);
    static Device fromJsonStr(const QByteArray& ba);

    static QJsonArray pathsToJson(const QList<DevicePath>& devicePaths);
    static QList<DevicePath> jsonToPaths(const QJsonArray& val);

    std::optional<DevicePath> findPath(const QUuid& id) const;
    DevicePath* getPathPtr(const QUuid& id);
    std::optional<QUuid> getBestPathId();
    static std::optional<QUuid> getBestPathId(const Device& dev);
    std::optional<QUuid> getRemoteOnlyPath() const;

    QString toString() const;
    QString toStringShort() const;
};

inline QDebug operator<< (QDebug d, const DevicePath& info) {
    d << info.toStringShort();
    return d;
}

inline QDebug operator<< (QDebug d, const Device& info) {
    d << info.toStringShort();
    return d;
}

inline QDebug operator<< (QDebug d, const QList<Device>& devList) {
    if (devList.isEmpty()) {
        d << QStringLiteral("No devices in list");
    }
    else {
        for (const auto& it: std::as_const(devList)) {
            d << it;
        }
    }
    return d;
}

inline QDebug operator<< (QDebug d, const QList<DevicePath>& pathList) {
    if (pathList.isEmpty()) {
        d << QStringLiteral("No path in list");
    }
    else {
        for (const auto& it: std::as_const(pathList)) {
            d << it;
        }
    }
    return d;
}

Q_DECLARE_METATYPE(DevicePath);
Q_DECLARE_METATYPE(Device);
