#pragma once

#include <QDateTime>
#include <QElapsedTimer>
#include <QObject>

namespace APP {

class SystemSleepMonitor : public QObject
{
    Q_OBJECT

public:
    explicit SystemSleepMonitor(QObject *parent = nullptr);

Q_SIGNALS:
    void processResumed(qint64 gapSeconds);

private:
    void tick();

    QDateTime _lastWallClock;
    QElapsedTimer _monotonic;
};

} // namespace APP
