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

    void clearAll();

    static DeviceList mergeDevices(const DeviceList& dev_1, const DeviceList& dev_2);
    static DeviceList build_devices(const QList<DevicePath>& records);


    void add_paths(const QList<DevicePath>& devs);
    void clear_paths();
    const QList<DevicePath>& paths() {return mergedPath_;}

signals:
    // something changed, call getDevices()
    void listUpdated();

private:
    mutable QReadWriteLock lock_;
    QList<DevicePath> mergedPath_;
};
