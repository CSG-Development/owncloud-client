#include "networkmonitor.h"

#include <QNetworkInterface>
#include <QLoggingCategory>


namespace APP {

Q_LOGGING_CATEGORY(lcNetworkMonitor, "sync.networkmonitor", QtInfoMsg)

NetworkMonitor::NetworkMonitor()
    : QObject(nullptr)
{
    qInfo(lcNetworkMonitor) << "NetworkMonitor created";
    _timer.setInterval(5000);

    connect(&_timer, &QTimer::timeout, this, [this] {
        const auto all = QNetworkInterface::allInterfaces();

        QSet<QString> actual;

        for (const auto& item: all) {
            if (!item.isValid() || !item.flags().testFlag(QNetworkInterface::IsRunning))
                continue;

            for (const auto& ip: item.addressEntries()) {
                if (ip.ip().protocol() == QAbstractSocket::IPv6Protocol)
                    continue;
                actual.insert(ip.ip().toString());
            }
        }

        if (isDiff(actual, ip_addresses)) {
            ip_addresses = actual;
            if (_startup) {
                qCInfo(lcNetworkMonitor) << "Network configuration startup";
                _startup = false;
            }
            else {
                qCInfo(lcNetworkMonitor) << "Network configuration changed";
                emit network_changed();
            }
        }
    });
}

bool NetworkMonitor::isDiff(const QSet<QString> &a, const QSet<QString> &b)
{
    if (a.size() != b.size())
        return true;

    return !(a-b).isEmpty();
}

NetworkMonitor *NetworkMonitor::instance()
{
    static NetworkMonitor instance;
    return &instance;
}

void NetworkMonitor::start()
{
    _timer.start();
}

void NetworkMonitor::emit_network_changed()
{
    emit network_changed();
}

bool NetworkMonitor::isWifiEthAvailable()
{
    return false;
    const auto all = QNetworkInterface::allInterfaces();

    for (const auto& item: all) {
        if (item.type() == QNetworkInterface::Wifi || item.type() == QNetworkInterface::Ethernet)
            return true;
    }

    return false;
}

} // namespace APP
