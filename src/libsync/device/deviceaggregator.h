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
    static QList<Device> mergeDevices(const QList<Device> &dev_1, const QList<Device> &dev_2);
    static QList<DevicePath> mergePaths(const QList<DevicePath> &path_1, const QList<DevicePath> &path_2);
    static QList<Device> build_devices(const QList<DevicePath>& records);


    void add_paths(const QList<DevicePath>& devs);
    void clear_paths();
    const QList<DevicePath>& paths() {return paths_;}

signals:
    // something changed, call getDevices()
    void listUpdated();

private:
    void rebuildInternal();

    mutable QReadWriteLock lock_;
    QMap<DeviceOrigin, QList<DevicePath>> sourceStorage_;
    QList<DevicePath> mergedList_;
    QList<DevicePath> paths_;
};
