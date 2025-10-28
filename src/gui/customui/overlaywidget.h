#pragma once

#include <QWidget>

class OverlayWidget : public QWidget
{
    void newParent();

public:
    explicit OverlayWidget(QWidget* parent = nullptr);

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;
    bool event(QEvent* ev) override;
};
