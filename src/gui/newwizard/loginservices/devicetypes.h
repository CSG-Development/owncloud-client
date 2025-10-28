#pragma once

#include <QString>
#include <QList>

enum class DeviceType {
    Unknown,
    Local,
    Public,
    Remote
};

class DevicePath
{
public:
    QString address;
    DeviceType deviceType = DeviceType::Unknown;
    int port = 0;

    static DeviceType strToDevType(const QString& str);
};

class Device
{
public:
    QString seagateDeviceID;
    QString certificateCommonName;
    QString friendlyName;
    QString hostname;

    QList<DevicePath> paths;

    void addPath(const QString& deviceID,  const DevicePath& path);
};

class DeviceInfo
{
public:
    QString name;
    QString host;
    int port = 0;
};
