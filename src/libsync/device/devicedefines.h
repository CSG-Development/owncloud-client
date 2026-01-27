#pragma once

#include "curatorlib.h"
#include <QList>
#include <QDebug>
#include <QDateTime>

// Ordered by connection priority
enum class DeviceType : quint8 {
    Unknown = 0,
    Local,
    Public,
    Remote
};

enum class DeviceOrigin {
    Unknown,
    Remote,
    MDNS,
    Static
};

class CURATORSYNC_EXPORT DeviceHardwareInfo
{
public:
    qint64 memory = 0;
    qint64 processor_count = 0;
    QString processor_type;
    QString toString() const;
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
    QString toString() const;
    QString toStringShort() const;
};

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

class CURATORSYNC_EXPORT DevHelpers
{
public:
    static DeviceType strToDevType(const QString& str);
    static QString devTypeToStr(DeviceType val);
    static DeviceOrigin strToDevOrigin(const QString& str);
    static QString originToStr(DeviceOrigin val);

    static QString makeServerUrl(const QString &url, int port, bool add_folder, bool add_port);
    static QUrl makePhotosUrl(const QUrl& other);
};

inline QDebug operator<< (QDebug d, const DeviceHardwareInfo& info) {
    d << info.toString();
    return d;
}

inline QDebug operator<< (QDebug d, const DeviceInfoAbout& info) {
    d << info.toStringShort();
    return d;
}

inline QDebug operator<< (QDebug d, const DeviceInfoStatus& info) {
    d << info.toString();
    return d;
}

Q_DECLARE_METATYPE(DeviceInfoAbout);
Q_DECLARE_METATYPE(DeviceInfoStatus);

