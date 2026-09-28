#include "arrow_toolbutton.h"
#include "apppalette.h"
#include "theme.h"
#include "stylehelper.h"

#include <QPainter>
#include <QMouseEvent>
#include <QStyle>
#include <QPainterPath>

namespace {
#if defined Q_OS_MACOS
constexpr qreal focusRadius = 0;
constexpr qreal focusWidth = 3;
constexpr qreal frameRadius = 0;
constexpr qreal frameWidth = 0;
constexpr qreal focusFrameMargin = 5;
#else
constexpr qreal focusRadius = 5;
constexpr qreal focusWidth = 2;
constexpr qreal frameRadius = 4;
constexpr qreal frameWidth = 1;
constexpr qreal focusFrameMargin = 5;
#endif
}

namespace APP {

ArrowToolButton::ArrowToolButton(QWidget *parent)
    : QToolButton(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setCursor(Qt::PointingHandCursor);
}

void ArrowToolButton::paintEvent(QPaintEvent* /*event*/)
{
    isDark = Theme::instance()->isDarkTheme();

    QPainter painter(this);

    QColor bgColor;
    QColor frameColor;

    if (!isEnabled()) {
        bgColor = QColor(buttonBackgroundDisabled());
        frameColor = QColor(buttonFrameDisabled());
    }
    else if (isPressed) {
        // Pressed
        bgColor = QColor(buttonBackgroundPressed());
        frameColor = QColor(buttonFramePressed());
    }
    else if (isHovered) {
        bgColor = QColor(buttonBackgroundHovered());
        frameColor = QColor(buttonFrameHovered());
    }
    else {
        bgColor = buttonBackgroundNormal();
        frameColor = buttonFrameNormal();
    }

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF roundRect = rect().adjusted(::focusFrameMargin, ::focusFrameMargin, -(::focusFrameMargin), -(::focusFrameMargin));
    QPainterPath path;

#ifdef Q_OS_MACOS
    path.addRect(roundRect);

    if (hasFocus()) {
        painter.setPen(QPen(buttonFrameFocused(), ::focusWidth, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
        painter.drawRect(rect());
    }
#else
    path.addRoundedRect(roundRect, ::frameRadius, ::frameRadius);

    if (hasFocus()) {
        painter.setPen(QPen(buttonFrameFocused(), ::focusWidth, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
        painter.drawRoundedRect(
            rect().adjusted(::focusWidth+1, ::focusWidth+1, -(::focusWidth+1), -(::focusWidth+1)),
            ::focusRadius, ::focusRadius);
    }
#endif

#ifdef Q_OS_MACOS
#endif

    painter.fillPath(path, bgColor);

#ifdef Q_OS_MACOS
    painter.setPen(QPen(frameColor, ::frameWidth, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
#else
    painter.setPen(QPen(frameColor, ::frameWidth, Qt::SolidLine));
#endif
    painter.drawPath(path);

    painter.restore();

    QRect pix_r = rect();
    QRect text_r = pix_r;

    constexpr QSize iconSize = {16, 17};
    pix_r = QRect(pix_r.right() - iconSize.width() - 11, (pix_r.height() - iconSize.height()) / 2, iconSize.width(), iconSize.height());
    const auto icon = StyleHelper::getArrowIcon(arrowType(), isPressed, !isEnabled(), isDark);
    if (!icon.isNull()) {
        icon.paint(&painter, pix_r, Qt::AlignCenter);
    }

    int alignment = Qt::TextShowMnemonic;
    alignment |= Qt::AlignCenter;

    QFontMetrics fontMetrics = painter.fontMetrics();
    const QString elidedLine = fontMetrics.elidedText(text(), Qt::ElideMiddle, text_r.width());

    text_r.adjust(0, 0, -iconSize.width(), 0);
    style()->drawItemText(&painter, QStyle::visualRect(layoutDirection(), rect(), text_r), alignment, palette(), isEnabled(), elidedLine);
}

void ArrowToolButton::enterEvent(QEnterEvent *event)
{
    isHovered = true;
    QToolButton::enterEvent(event);
}

void ArrowToolButton::leaveEvent(QEvent *event)
{
    isHovered = false;
    QToolButton::leaveEvent(event);
}

void ArrowToolButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isPressed = true;
    }
    QToolButton::mousePressEvent(event);
}

void ArrowToolButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isPressed = false;
    }
    QToolButton::mouseReleaseEvent(event);
}

#ifdef Q_OS_MACOS
QColor ArrowToolButton::buttonFrameFocused() const {return AppPalette::color(ColorToken::PushButtonFocusRing, isDark);}

QColor ArrowToolButton::buttonFrameNormal() const  {return AppPalette::color(ColorToken::MacButtonFrameNormal, isDark);}
QColor ArrowToolButton::buttonFramePressed() const {return AppPalette::color(ColorToken::MacButtonFramePressed, isDark);}
QColor ArrowToolButton::buttonFrameHovered() const {return AppPalette::color(ColorToken::MacButtonFrameHovered, isDark);}
QColor ArrowToolButton::buttonFrameDisabled() const {return AppPalette::color(ColorToken::MacArrowButtonFrameDisabled, isDark);}

QColor ArrowToolButton::buttonBackgroundNormal() const {return AppPalette::color(ColorToken::MacArrowButtonBackgroundNormal, isDark);}
QColor ArrowToolButton::buttonBackgroundPressed() const {return AppPalette::color(ColorToken::MacButtonBackgroundPressed, isDark);}
QColor ArrowToolButton::buttonBackgroundHovered() const {return AppPalette::color(ColorToken::MacButtonBackgroundHovered, isDark);}
QColor ArrowToolButton::buttonBackgroundDisabled() const {return AppPalette::color(ColorToken::MacButtonBackgroundDisabled, isDark);}
#else
QColor ArrowToolButton::buttonFrameFocused() const {return AppPalette::color(ColorToken::ButtonFrameFocused, isDark);}

QColor ArrowToolButton::buttonFrameNormal() const  {return AppPalette::color(ColorToken::ButtonFrameNormal, isDark);}
QColor ArrowToolButton::buttonFramePressed() const {return AppPalette::color(ColorToken::ButtonFramePressed, isDark);}
QColor ArrowToolButton::buttonFrameHovered() const {return AppPalette::color(ColorToken::ButtonFrameHovered, isDark);}
QColor ArrowToolButton::buttonFrameDisabled() const {return AppPalette::color(ColorToken::ButtonFrameDisabled, isDark);}

QColor ArrowToolButton::buttonBackgroundNormal() const {return AppPalette::color(ColorToken::ButtonBackgroundNormal, isDark);}
QColor ArrowToolButton::buttonBackgroundPressed() const {return AppPalette::color(ColorToken::ButtonBackgroundPressed, isDark);}
QColor ArrowToolButton::buttonBackgroundHovered() const {return AppPalette::color(ColorToken::ButtonBackgroundHovered, isDark);}
QColor ArrowToolButton::buttonBackgroundDisabled() const {return AppPalette::color(ColorToken::ArrowButtonBackgroundDisabled, isDark);}
#endif

} // namespace APP
