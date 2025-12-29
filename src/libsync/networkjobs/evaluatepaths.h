#pragma once

#include "curatorlib.h"
#include "device/devicetypes.h"
#include "accessmanager.h"

#include <QObject>
#include <QNetworkAccessManager>
#include <QRestAccessManager>
#include <QNetworkRequestFactory>

class Device;

namespace CUR {

class DeviceApi;

class CURATORSYNC_EXPORT EvaluatePath: public QObject
{
    Q_OBJECT

public:
    explicit EvaluatePath(QObject* parent = nullptr);

    void start_evaluate(Device* dev);
    bool isRunning() const {return _isRunning;}

signals:
    void path_evaluated(const QUuid& id, bool success);
    void evaluate_finished();

private:
    void evaluate(const QUuid& id);
    void query_device_status(const DevicePath& dpath);

private:
    DeviceApi* devApi = nullptr;
    Device* device_ = nullptr;
    QList<QUuid> id_queue;
    bool _isRunning = false;
    QUuid current_id;
};

} // namespace CUR
