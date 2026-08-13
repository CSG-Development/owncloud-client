#include "systemsleepmonitor.h"

#include <QLoggingCategory>
#include <QTimer>

namespace {
constexpr int tickIntervalMsC = 5000;
constexpr qint64 gapThresholdSecondsC = 30;
}

namespace APP {

Q_LOGGING_CATEGORY(lcSleepMonitor, "gui.sleepmonitor", QtInfoMsg)

SystemSleepMonitor::SystemSleepMonitor(QObject *parent)
    : QObject(parent)
{
    _lastWallClock = QDateTime::currentDateTimeUtc();
    _monotonic.start();

    auto *timer = new QTimer(this);
    timer->setInterval(tickIntervalMsC);
    connect(timer, &QTimer::timeout, this, &SystemSleepMonitor::tick);
    timer->start();
}

void SystemSleepMonitor::tick()
{
    const auto now = QDateTime::currentDateTimeUtc();
    const qint64 wallGap = _lastWallClock.secsTo(now);
    const qint64 monotonicGap = _monotonic.restart() / 1000;
    _lastWallClock = now;

    if (wallGap < gapThresholdSecondsC) {
        return;
    }

    qCInfo(lcSleepMonitor) << "The process did not run for" << wallGap << "seconds of wall clock time and"
                           << monotonicGap << "seconds of monotonic time";
    Q_EMIT processResumed(wallGap);
}

} // namespace APP
