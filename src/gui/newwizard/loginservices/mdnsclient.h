#pragma once

#include "devicetypes.h"

#include <QObject>
#include <QDnsLookup>

namespace CUR {

class MdnsClient: public QObject
{
    Q_OBJECT

public:
    explicit MdnsClient(QObject* parent = nullptr);
    ~MdnsClient();

    void query();

    [[nodiscard]] const QList<DeviceInfo>& records() {return records_;}

signals:
    void requestCompleted();

private:
    QDnsLookup* dns = nullptr;
    QList<DeviceInfo> records_;
};

} // namespace CUR
