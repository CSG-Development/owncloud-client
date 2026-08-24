#pragma once

#include "floatingbottombanner.h"

namespace Ui {
class DeviceUnreachableBanner;
}

class DeviceUnreachableBanner : public FloatingBottomBanner
{
    Q_OBJECT

public:
    explicit DeviceUnreachableBanner(QWidget *parent = nullptr);
    ~DeviceUnreachableBanner() override;

signals:
    void retryClicked();

protected:
    void refreshGeometryForParent() override;

private:
    void updateStyles(bool isDark);

    Ui::DeviceUnreachableBanner *ui = nullptr;
};
