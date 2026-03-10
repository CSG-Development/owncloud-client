#pragma once

#include "personalcloudlib.h"

#include <QSet>
#include <QObject>
#include <QTimer>

namespace APP {

class APPLICATIONSYNC_EXPORT NetworkMonitor: public QObject
{
    Q_OBJECT

public:
    static NetworkMonitor *instance();
    ~NetworkMonitor() override {}

    void start();

    void emit_network_changed();

    static bool isWifiEthAvailable();

signals:
    void network_changed();

private:
    NetworkMonitor();

    bool isDiff(const QSet<QString>& a, const QSet<QString>& b);

private:
    QSet<QString> ip_addresses;
    QTimer _timer;
    bool _startup = true;
};

} // namespace APP
