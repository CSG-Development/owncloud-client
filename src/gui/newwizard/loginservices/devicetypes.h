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

    static std::optional<DevicePath> firstRemotePath(const Device& dev);
};

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

class DeviceInfoAbout
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

    static DeviceInfoAbout fromJson(const QJsonDocument& doc);
};

inline QDebug operator<< (QDebug d, const DeviceInfoAbout& info) {
    d << QStringLiteral("cert: %1, hostname: %2, serial: %3")
             .arg(info.certificate_common_name)
             .arg(info.hostname)
             .arg(info.serial_number);
    return d;
}

class DeviceInfoStatus
{
public:
    bool oobe_done = false;
    QString app_files;
    QString app_photos;
    QString state;

    static DeviceInfoStatus fromJson(const QJsonDocument& doc);
};

inline QDebug operator<< (QDebug d, const DeviceInfoStatus& info) {
    d << QStringLiteral("oobe: %1, state: %2")
            .arg(info.oobe_done)
            .arg(info.state);
    return d;
}

class MdnsRecord
{
public:
    QString name;
    QString host;
    int port = 0;
    DeviceInfoAbout about;
    DeviceInfoStatus status;
};

class DeviceInfo
{
public:
    DeviceInfoAbout about;
    DeviceInfoStatus status;
    QString name;
    QString host;
    int port = 0;
    DeviceType deviceType = DeviceType::Unknown;
    int tempId = -1;

    static void assignIds(QList<DeviceInfo>& list) {
        int id = 0;
        for (auto& item: list) {
            item.tempId = id;
            id++;
        }
    }
};

QString normalizeUrl(const QString &url, int port, bool add_folder);

Q_DECLARE_METATYPE(DeviceInfo);
Q_DECLARE_METATYPE(Device);

