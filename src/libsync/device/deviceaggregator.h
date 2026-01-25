#include "devicetypes.h"

#include <QMap>
#include <QList>
#include <QObject>
#include <QReadWriteLock>

class DeviceAggregator : public QObject
{
    Q_OBJECT

public:
    explicit DeviceAggregator(QObject* parent = nullptr);

    // safe
    QList<DevicePath> getDevicePaths() const;

    // safe add
    void updateSource(DeviceOrigin origin, const QList<DevicePath>& newDevices);
    void clearAll();

    static void merge(Device &target, const QList<DevicePath> &path_sources);
    static QList<Device> build_devices(const QList<DevicePath>& records);


signals:
    // something changed, call getDevices()
    void listUpdated();

private:
    void rebuildInternal();

    mutable QReadWriteLock lock_;
    QMap<DeviceOrigin, QList<DevicePath>> sourceStorage_;
    QList<DevicePath> mergedList_;
};
