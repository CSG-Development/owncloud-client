#pragma once

#include "curatorlib.h"

#include <QString>
#include <QList>
#include <QDateTime>
#include <QUuid>

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
    DevicePath() = delete;
    DevicePath(const QString& addr, DeviceType devType, DevicePathOrigin org, int pport);

    QString address;
    DeviceType deviceType = DeviceType::Unknown;
    DevicePathOrigin origin = DevicePathOrigin::Unknown;    // Mostly for debug
    int port = 0;

    bool isOnline = false;  // Not saved in config, runtime only
    bool isActive = false;  // Not saved in config, runtime only
    QUuid id;               // Not saved in config, runtime only

    static DeviceType strToDevType(const QString& str);
    static QString devTypeToStr(DeviceType val);
    static DevicePathOrigin strToDevOrigin(const QString& str);
    static QString originToStr(DevicePathOrigin val);

    QJsonObject toJson() const;
    static DevicePath fromJson(const QJsonObject &val);

    QString toString() const;
};

inline QDebug operator<< (QDebug d, const DevicePath& info) {
    d << info.toString();
    return d;
}

class CURATORSYNC_EXPORT Device
{
private:
    QString _friendlyName;

public:
    QString seagateDeviceID;
    QString certificateCommonName;
    QString hostname;
    bool isStatic = false;
    QList<DevicePath> paths;

    void setFriendlyName(const QString& name);
    QString friendlyName() const {return _friendlyName;}

    static Device MakeStatic(const QString& url, const QString& name);

    static QJsonDocument toJson(const Device& dev);
    static QByteArray toJsonStr(const Device& dev);

    static Device fromJson(const QJsonDocument &obj);
    static Device fromJsonStr(const QByteArray& ba);

    static QJsonArray pathsToJson(const QList<DevicePath>& devicePaths);
    static QList<DevicePath> jsonToPaths(const QJsonArray& val);

    void setActicvePath(const QUuid& id);
    void setOnlinePath(const QUuid& id);
    void clearOnlinePaths();

    static QString makeServerUrl(const QString &url, int port, bool add_folder, bool add_port);

    std::optional<DevicePath> findPath(const QUuid& id) const;
    std::optional<QUuid> getBestPathId();
    static std::optional<QUuid> getBestPathId(const Device& dev);

    QString toString() const;
};

inline QDebug operator<< (QDebug d, const Device& info) {
    d << info.toString();
    return d;
}

class CURATORSYNC_EXPORT DeviceHardwareInfo
{
public:
    qint64 memory = 0;
    qint64 processor_count = 0;
    QString processor_type;
    QString toString() const;
};

inline QDebug operator<< (QDebug d, const DeviceHardwareInfo& info) {
    d << info.toString();
    return d;
}

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
    QString toString() const;
};

inline QDebug operator<< (QDebug d, const DeviceInfoAbout& info) {
    d << info.toString();
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
    QString toString() const;
};

inline QDebug operator<< (QDebug d, const DeviceInfoStatus& info) {
    d << info.toString();
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
    QString toString() const;
};

class CURATORSYNC_EXPORT DeviceInfo
{
public:
    DeviceInfoAbout about;
    DeviceInfoStatus status;
    QString certName;
    QString friendlyName;
    QString host;
    int port = 0;
    DeviceType deviceType = DeviceType::Unknown;
    int tempId = -1;

    static void assignIds(QList<DeviceInfo>& list);

    QString toString() const;
};

inline QDebug operator<< (QDebug d, const DeviceInfo& info) {
    d << info.toString();
    return d;
}

Q_DECLARE_METATYPE(DeviceInfo);
Q_DECLARE_METATYPE(Device);

