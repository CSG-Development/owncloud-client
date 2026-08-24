#pragma once

#include <QWidget>

// Floats a widget centered horizontally, pinned near the bottom of its
// parent, and keeps it there as the parent resizes/moves/shows. Shared by
// ToastWidget and DeviceUnreachableBanner.
class FloatingBottomBanner : public QWidget
{
    Q_OBJECT

public:
    explicit FloatingBottomBanner(QWidget *parent = nullptr);
    ~FloatingBottomBanner() override;

protected:
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void showEvent(QShowEvent *event) override;

    virtual void refreshGeometryForParent();

private:
    void bindParent();
    void unbindParent();
};
