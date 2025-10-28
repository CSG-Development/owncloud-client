#include "devicetypes.h"

#include <QMap>

void Device::addPath(const QString &deviceID, const DevicePath &path)
{

}

DeviceType DevicePath::strToDevType(const QString &str)
{
    QMap<QString, DeviceType> map = {
        {QStringLiteral("local"), DeviceType::Local},
        {QStringLiteral("public"), DeviceType::Public},
        {QStringLiteral("remote"), DeviceType::Remote}
    };

    if (map.contains(str))
        return map[str];
    return DeviceType::Unknown;
}
