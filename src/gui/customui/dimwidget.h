#pragma once

#include "overlaywidget.h"
#include <QWidget>

class DimWidget: public OverlayWidget
{
    Q_OBJECT

public:
    explicit DimWidget(QWidget* parent = nullptr);
    void setRounded(int radius);

protected:
    void paintEvent(QPaintEvent*) override final;

private:
    int radius_ = 0;
};
