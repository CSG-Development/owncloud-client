#include "onboardingindicator.h"

#include "theme.h"

#include <QColor>
#include <QPainter>
#include <QSize>
#include <QSizePolicy>
#include <QtGlobal>

namespace
{
constexpr int pointCellSize = 12;
constexpr int currentPointSize = 6;
constexpr int pointSize = 4;
constexpr int lightThemePointColorValue = 0;
constexpr int darkThemePointColorValue = 255;
constexpr qreal lightThemePointAlpha = 0.44;
constexpr qreal darkThemePointAlpha = 0.54;
}

OnboardingIndicator::OnboardingIndicator(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(APP::Theme::instance(), &APP::Theme::themeChanged, this, [this] {
        update();
    });
}

int OnboardingIndicator::count() const
{
    return _count;
}

void OnboardingIndicator::setCount(int count)
{
    const int normalizedCount = qMax(0, count);
    if (_count == normalizedCount) {
        return;
    }

    _count = normalizedCount;
    _currentIndex = _count > 0 ? qBound(0, _currentIndex, _count - 1) : 0;
    updateGeometry();
    update();
}

int OnboardingIndicator::currentIndex() const
{
    return _currentIndex;
}

void OnboardingIndicator::setCurrentIndex(int index)
{
    const int normalizedIndex = _count > 0 ? qBound(0, index, _count - 1) : 0;
    if (_currentIndex == normalizedIndex) {
        return;
    }

    _currentIndex = normalizedIndex;
    update();
}

QSize OnboardingIndicator::sizeHint() const
{
    return QSize(_count * pointCellSize, pointCellSize);
}

QSize OnboardingIndicator::minimumSizeHint() const
{
    return sizeHint();
}

void OnboardingIndicator::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(pointColor());

    for (int index = 0; index < _count; ++index) {
        const int diameter = index == _currentIndex ? currentPointSize : pointSize;
        const QRectF cell(index * pointCellSize, 0, pointCellSize, pointCellSize);
        const QPointF center = cell.center();
        const QRectF point(center.x() - diameter / 2.0, center.y() - diameter / 2.0, diameter, diameter);
        painter.drawEllipse(point);
    }
}

QColor OnboardingIndicator::pointColor() const
{
    const bool isDarkTheme = APP::Theme::instance()->isDarkTheme();
    const int colorValue = isDarkTheme ? darkThemePointColorValue : lightThemePointColorValue;

    QColor color(colorValue, colorValue, colorValue);
    color.setAlphaF(isDarkTheme ? darkThemePointAlpha : lightThemePointAlpha);
    return color;
}
