#include "progressindicator.h"
#include "theme.h"

#include <QPainter>
#include <QTimer>

namespace {
QPair<QColor,QColor> fgColor = {QColor("#1976D2"), QColor("#64B5F6")};
QPair<QColor,QColor> bgColor = {QColor("#E0E0E0"), QColor("#616161")};
}

ProgressIndicator::ProgressIndicator(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(48, 48);

    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &ProgressIndicator::updateAnimation);
    timer_->start(16); // ~60 FPS

    startTime_ = QTime::currentTime();
    easingCurve_.setType(QEasingCurve::InOutQuad);
}

void ProgressIndicator::setDarkTheme()
{
    isDark = CUR::Theme::instance()->isDarkTheme();
    update();
}

void ProgressIndicator::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    flipHorizontal(painter, rect());    // CW rotarion

    int size = qMin(width(), height());
    double thickness = size * 0.1;

    QPen pen;
    pen.setWidthF(thickness);
    pen.setCapStyle(Qt::RoundCap);
    pen.setColor(isDark ? bgColor.second : bgColor.first);
    painter.setPen(pen);
    painter.drawEllipse(QRectF(thickness/2, thickness/2, width() - thickness, height() - thickness));

    int elapsed = startTime_.msecsTo(QTime::currentTime());
    float progress = fmod(elapsed / 1000.0, 2.0) / 2.0;

    float easedProgress = easingCurve_.valueForProgress(progress);

    double startAngle = easedProgress * 360 * 2;
    double spanAngle = 90 + 45 * sin(progress * M_PI * 2);

    pen.setColor(isDark ? fgColor.second : fgColor.first);
    painter.setPen(pen);

    painter.drawArc(QRectF(thickness/2, thickness/2, width() - thickness, height() - thickness), startAngle * 16, spanAngle * 16);
}

void ProgressIndicator::flipHorizontal(QPainter &painter, const QRect &r)
{
    QTransform transform(painter.transform());
    qreal m11 = transform.m11();
    qreal m12 = transform.m12();
    qreal m13 = transform.m13();
    qreal m21 = transform.m21();
    qreal m22 = transform.m22();
    qreal m23 = transform.m23();
    qreal m31 = transform.m31();
    qreal m32 = transform.m32();
    qreal m33 = transform.m33();
    qreal scale = m11;

    // Horizontal flip
    m11 = -m11;
    if (m31 > 0)
        m31 = 0;
    else
        m31 = (r.width() * scale);

    transform.setMatrix(m11, m12, m13, m21, m22, m23, m31, m32, m33);
    painter.setTransform(transform);
}

void ProgressIndicator::updateAnimation()
{
    update();
}
