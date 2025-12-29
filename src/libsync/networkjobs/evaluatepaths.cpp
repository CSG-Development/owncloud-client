#include "evaluatepaths.h"
#include "simpleresolveurljobfactory.h"
#include "determineauthtypejobfactory.h"
#include "device/deviceapi.h"

#include <QLoggingCategory>
#include <QRestReply>
#include <QJsonDocument>

namespace {
const QString api_dev_status = QStringLiteral("/api/v1/status"); // GET
}

namespace CUR {

Q_LOGGING_CATEGORY(lcEvaluator, "device.evaluaror", QtInfoMsg)

EvaluatePath::EvaluatePath(QObject *parent)
    : QObject(parent)
    , devApi(new DeviceApi(this))
{
}

void EvaluatePath::start_evaluate(Device *dev)
{
    qCDebug(lcEvaluator) << "Starting device path evaluate";
    if (isRunning()) {
        qCWarning(lcEvaluator) << "Already running";
        emit evaluate_finished();
    }

    disconnect(this, &EvaluatePath::path_evaluated, this, nullptr);
    disconnect(devApi, &DeviceApi::status_finished, this, nullptr);
    current_id = QUuid();

    _isRunning = true;
    device_ = dev;
    device_->clearOnlinePaths();

    if (device_->paths.isEmpty()) {
        qCWarning(lcEvaluator) << "Path list is empty, evaluating finished";
        _isRunning = false;
        emit evaluate_finished();
        return;
    }

    for (const auto& p: std::as_const(device_->paths)) {
        id_queue.append(p.id);
    }

    connect(this, &EvaluatePath::path_evaluated, this, [&](const QUuid& id, bool success) {

        if (success)
            device_->setOnlinePath(id);

        if (id_queue.isEmpty()) {
            disconnect(this, &EvaluatePath::path_evaluated, this, nullptr);
            qCInfo(lcEvaluator) << "Evaluating finished";
            _isRunning = false;
            Q_EMIT evaluate_finished();
            return;
        }

        current_id = id_queue.takeFirst();
        evaluate(current_id);
    });

    connect(devApi, &DeviceApi::status_finished, this, [&](const DeviceInfoStatus& info, int code) {
        bool status_ok = false;
        if (code == 200) {
            if (info.oobe_done)
                status_ok = true;
        }
        Q_EMIT path_evaluated(current_id, status_ok);
    });

    current_id = id_queue.takeFirst();
    evaluate(current_id);
}

void EvaluatePath::evaluate(const QUuid &id)
{
    auto dev_path = device_->findPath(id);
    if (!dev_path) {
        Q_EMIT path_evaluated(id, false);
        return;
    }

    query_device_status(dev_path.value());
}

void EvaluatePath::query_device_status(const DevicePath& dpath)
{
    auto url = Device::makeServerUrl(dpath.address, dpath.port, false, true);
    qCDebug(lcEvaluator) << "Query remote status" << url;
    devApi->query_device_status(url);
}


} // namespace CUR
