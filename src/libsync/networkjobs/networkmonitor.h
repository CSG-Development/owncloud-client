#pragma once

#include "curatorlib.h"

#include <QSet>
#include <QObject>
#include <QTimer>

namespace CUR {

class CURATORSYNC_EXPORT NetworkMonitor: public QObject
{
    Q_OBJECT

public:
    static NetworkMonitor *instance();
    ~NetworkMonitor() override {}

    void start();

signals:
    void network_changed();

private:
    NetworkMonitor();

    bool isDiff(const QSet<QString>& a, const QSet<QString>& b);

private:
    QSet<QString> ip_addresses;
    QTimer _timer;
};

} // namespace CUR
