#pragma once

#include "curatorlib.h"
#include "devicetypes.h"
#include "accessmanager.h"
#include <QObject>

class Device;

namespace CUR {

class CURATORSYNC_EXPORT EvaluatePath: public QObject
{
    Q_OBJECT

public:
    explicit EvaluatePath(QObject* parent = nullptr);

    void start_evaluate(Device* dev);

signals:
    void path_evaluated(const QUuid& id, bool success);
    void evaluate_finished();

private:
    void evaluate(const QUuid& id);
    void resetAccessManager();

private:
    Device* device_ = nullptr;
    QList<QUuid> id_queue;
    AccessManager* mgr = nullptr;
};

} // namespace CUR
