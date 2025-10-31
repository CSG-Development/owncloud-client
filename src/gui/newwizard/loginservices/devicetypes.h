#pragma once

#include <QString>
#include <QList>
#include <QDateTime>

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
    int tempId = -1;

    bool isEqual(const DeviceInfo& di) const {
        return tempId == di.tempId && name == di.name && host == di.host && port == di.port;
    }
    static void assignIds(QList<DeviceInfo>& list) {
        int id = 0;
        for (auto& item: list) {
            item.tempId = id;
            id++;
        }
    }
};

Q_DECLARE_METATYPE(DeviceInfo);

inline bool operator==(const DeviceInfo& lhs, const DeviceInfo& rhs)
{
    return lhs.isEqual(rhs);
}

class DeviceHardwareInfo
{
public:
    qint64 memory = 0;
    qint64 processor_count = 0;
    QString processor_type;
};

class DeviceIpv4Info {
public:
    QString gateway;
    QString ipv4;
    QString netmask;
};

class LocalDeviceInterface
{
public:
    DeviceIpv4Info ipv4_info;
    QString link;
    QString mac_address;
    QString name;
    QString type;
};

class LocalDeviceInfo
{
public:
    QString certificate_common_name;
    QDateTime date;
    QString default_mac_addr;
    QString hostname;
    QString install_id;
    QString model_name;
    QString model_number;
    QString os_state;
    QString product_id;
    QString serial_number;
    QString version;
    DeviceHardwareInfo hardware_info;
    QList<LocalDeviceInterface> network_interfaces;
};
