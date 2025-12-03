#include "evaluatepaths.h"
#include "simpleresolveurljobfactory.h"
#include "determineauthtypejobfactory.h"

#include <QLoggingCategory>

namespace CUR {

Q_LOGGING_CATEGORY(lcEvaluator, "device.evaluaror", QtInfoMsg)

EvaluatePath::EvaluatePath(QObject *parent)
    : QObject(parent)
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
            qCWarning(lcEvaluator) << "Evaluating finished";
            _isRunning = false;
            Q_EMIT evaluate_finished();
            return;
        }

        auto curr_id = id_queue.takeFirst();
        evaluate(curr_id);
    });

    auto curr_id = id_queue.takeFirst();
    evaluate(curr_id);
}

void EvaluatePath::evaluate(const QUuid &id)
{
    auto dev_path = device_->findPath(id);
    if (!dev_path) {
        Q_EMIT path_evaluated(id, false);
        return;
    }

    QUrl serverUrl = QUrl(Device::makeServerUrl(dev_path->address, dev_path->port, true, (dev_path->origin == DevicePathOrigin::MDNS) ? false : true));

    if (!serverUrl.isValid()) {
        Q_EMIT path_evaluated(id, false);
        return;
    }

    resetAccessManager();

    // first, we must resolve the actual server URL
    auto resolveJob = SimpleResolveUrlJobFactory(mgr).startJob(serverUrl, this);

    connect(resolveJob, &CoreJob::finished, resolveJob, [this, resolveJob, dev_id = id]() {
        resolveJob->deleteLater();

        if (!resolveJob->success()) {
            Q_EMIT path_evaluated(dev_id, false);
            return;
        }

        const auto resolvedUrl = resolveJob->result().toUrl();

        // next, we need to find out which kind of authentication page we have to present to the user
        auto authTypeJob = DetermineAuthTypeJobFactory(mgr).startJob(resolvedUrl, this);

        connect(authTypeJob, &CoreJob::finished, authTypeJob, [this, authTypeJob, resolvedUrl, dev_id]() {
            authTypeJob->deleteLater();

            if (authTypeJob->result().isNull()) {
                Q_EMIT path_evaluated(dev_id, false);
                return;
            }

            Q_EMIT path_evaluated(dev_id, true);
        });
    });

    connect(resolveJob, &CoreJob::caCertificateAccepted, this, [this](const QSslCertificate &caCertificate) {
        mgr->addCustomTrustedCaCertificates({caCertificate});
    }, Qt::DirectConnection);
}

void EvaluatePath::resetAccessManager()
{
    if (mgr != nullptr) {
        mgr->deleteLater();
    }

    mgr = new AccessManager(this);
    mgr->setTransferTimeout(3 * 1000);
}

} // namespace CUR
