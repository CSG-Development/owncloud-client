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
    DevicePath(const QString& addr, DeviceType devType, DevicePathOrigin org, int pport)
        : address(addr)
        , deviceType(devType)
        , origin(org)
        , port(pport)
    {
        id = QUuid::createUuid();
    }

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

    QString toString() const {
        QStringList l;
        l << QStringLiteral("DevicePath{");
        l << QStringLiteral("address:%1,").arg(address);
        l << QStringLiteral("port:%1,").arg(port);
        l << QStringLiteral("deviceType:%1,").arg(devTypeToStr(deviceType));
        l << QStringLiteral("origin:%1,").arg(originToStr(origin));
        l << QStringLiteral("online:%1,").arg(isOnline);
        l << QStringLiteral("active:%1,").arg(isActive);
        l << QStringLiteral("id:%1").arg(id.toString());
        l << QStringLiteral("}");
        return l.join(QStringLiteral(""));
    }
};

inline QDebug operator<< (QDebug d, const DevicePath& info) {
    d << info.toString();
    return d;
}

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
    static QString normalizeUrl(const QString &url, int port, bool add_folder);

    std::optional<DevicePath> findPath(const QUuid& id) const;
    std::optional<QUuid> getBestPathId();
    static std::optional<QUuid> getBestPathId(const Device& dev);

    QString toString() const {
        QStringList l;
        l << QStringLiteral("Device{");
        l << QStringLiteral("seagateDeviceID:%1,").arg(seagateDeviceID);
        l << QStringLiteral("certificateCommonName:%1,").arg(certificateCommonName);
        l << QStringLiteral("friendlyName:%1,").arg(friendlyName);
        l << QStringLiteral("hostname:%1,").arg(hostname);
        l << QStringLiteral("isStatic:%1,").arg(isStatic);
        for (const auto& p: paths) {
            l << p.toString();
        }
        l << QStringLiteral("}");
        return l.join(QStringLiteral(""));
    }
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
    QString toString() const {
        QStringList l;
        l << QStringLiteral("DeviceHardwareInfo{");
        l << QStringLiteral("memory:%1,").arg(memory);
        l << QStringLiteral("processor_count:%1,").arg(processor_count);
        l << QStringLiteral("processor_type:%1").arg(processor_type);
        l << QStringLiteral("}");
        return l.join(QStringLiteral(""));
    }
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
    QString toString() const {
        QStringList l;
        l << QStringLiteral("DeviceInfoAbout{");
        l << QStringLiteral("cert_common_name:%1,").arg(certificate_common_name);
        l << QStringLiteral("date:%1,").arg(date.toString(QStringLiteral("yyyy-MM-dd hh:mm")));
        l << QStringLiteral("default_mac_addr:%1,").arg(default_mac_addr);
        l << QStringLiteral("hostname:%1,").arg(hostname);
        l << QStringLiteral("install_id:%1,").arg(install_id);
        l << QStringLiteral("model_name:%1,").arg(model_name);
        l << QStringLiteral("model_number:%1,").arg(model_number);
        l << QStringLiteral("os_state:%1,").arg(os_state);
        l << QStringLiteral("product_id:%1,").arg(product_id);
        l << QStringLiteral("serial_number:%1,").arg(serial_number);
        l << QStringLiteral("version:%1").arg(version);
        l << QStringLiteral("}");
        return l.join(QStringLiteral(""));
    }
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
    QString toString() const {
        return QStringLiteral("DeviceInfoStatus{oobe:%1,state:%2}").arg(oobe_done).arg(state);
    }
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

    QString toString() const {
        QStringList l;
        l << QStringLiteral("DeviceInfo{");
        l << QStringLiteral("name:%1,").arg(name);
        l << QStringLiteral("host:%1,").arg(host);
        l << QStringLiteral("port:%1,").arg(port);
        l << QStringLiteral("deviceType:%1,").arg(DevicePath::devTypeToStr(deviceType));
        l << QStringLiteral("status:%1").arg(status.toString());
        l << QStringLiteral("}");
        return l.join(QStringLiteral(""));
    }
};

inline QDebug operator<< (QDebug d, const DeviceInfo& info) {
    d << info.toString();
    return d;
}

Q_DECLARE_METATYPE(DeviceInfo);
Q_DECLARE_METATYPE(Device);

