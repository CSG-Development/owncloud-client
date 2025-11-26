#pragma once

#include "curatorlib.h"

#include <QString>
#include <QList>
#include <QDateTime>

// Ordered by connection priority
enum class DeviceType : quint8 {
    Unknown = 0,
    Local,
    Public,
    Remote
};

enum class DevicePathOrigin {
    Unknown,
    Remote,
    MDNS,
    Static
};

class CURATORSYNC_EXPORT DevicePath
{
public:
    QString address;
    DeviceType deviceType = DeviceType::Unknown;
    DevicePathOrigin origin = DevicePathOrigin::Unknown;    // Mostly for debug
    int port = 0;

    static DeviceType strToDevType(const QString& str);
    static QString devTypeToStr(DeviceType val);

    QJsonObject toJson() const;
    void fromJson(const QJsonObject &val);
};

class CURATORSYNC_EXPORT Device
{
public:
    QString seagateDeviceID;
    QString certificateCommonName;
    QString friendlyName;
    QString hostname;
    bool isStatic = false;
    QList<DevicePath> paths;

    static std::optional<DevicePath> firstRemotePath(const Device& dev);
    static std::optional<DevicePath> firstLocalPath(const Device& dev);
    static std::optional<DevicePath> getPath(const Device& dev, QList<DeviceType> types);
    static std::optional<DevicePath> getBestPath(const Device& dev);

    static Device MakeStatic(const QString& url, const QString& name);

    static QJsonDocument toJson(const Device& dev);
    static QByteArray toJsonStr(const Device& dev);

    static Device fromJson(const QJsonDocument &obj);
    static Device fromJsonStr(const QByteArray& ba);

    static QJsonArray pathsToJson(const QList<DevicePath>& devicePaths);
    static QList<DevicePath> jsonToPaths(const QJsonArray& val);

    static QString normalizeUrl(const QString &url, int port, bool add_folder);
};

class CURATORSYNC_EXPORT DeviceHardwareInfo
{
public:
    qint64 memory = 0;
    qint64 processor_count = 0;
    QString processor_type;
};

class CURATORSYNC_EXPORT DeviceIpv4Info {
public:
    QString gateway;
    QString ipv4;
    QString netmask;
};

class CURATORSYNC_EXPORT LocalDeviceInterface
{
public:
    DeviceIpv4Info ipv4_info;
    QString link;
    QString mac_address;
    QString name;
    QString type;
};

class CURATORSYNC_EXPORT DeviceInfoAbout
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

class CURATORSYNC_EXPORT DeviceInfoStatus
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

class CURATORSYNC_EXPORT MdnsRecord
{
public:
    QString name;
    QString host;
    int port = 0;
    DeviceInfoAbout about;
    DeviceInfoStatus status;
};

class CURATORSYNC_EXPORT DeviceInfo
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


Q_DECLARE_METATYPE(DeviceInfo);
Q_DECLARE_METATYPE(Device);

