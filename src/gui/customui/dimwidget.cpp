#include "dimwidget.h"

#include <QVBoxLayout>
#include <QFrame>
#include <QPainter>
#include <QPainterPath>

DimWidget::DimWidget(QWidget* parent)
    : OverlayWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
}

void DimWidget::setRounded(int radius)
{
    radius_ = radius;
}

void DimWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
     if (radius_ > 0)
     {
        QPainterPath pp;
        pp.addRoundedRect(rect(), radius_, radius_);
        p.fillPath(pp, QColor({0, 0, 0, 127}));
     }
     else
     {
        p.fillRect(rect(), {0, 0, 0, 127});
     }
}
