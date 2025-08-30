#include "arrow_toolbutton.h"
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

namespace CUR {

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
    QRect text_r = rect();

    auto pm = StyleHelper::getArrowPixmap(arrowType(), isPressed, !isEnabled(), isDark);
    pix_r.adjust(0, 0, -pm.width(), 0);
    style()->drawItemPixmap(&
        painter, pix_r, Qt::AlignRight|Qt::AlignVCenter, pm);

    int alignment = Qt::TextShowMnemonic;
    alignment |= Qt::AlignCenter;

    QFontMetrics fontMetrics = painter.fontMetrics();
    const QString elidedLine = fontMetrics.elidedText(text(), Qt::ElideMiddle, text_r.width());

    text_r.adjust(0, 0, -pm.width(), 0);
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
QColor ArrowToolButton::buttonFrameFocused() const {return isDark ? QColor(0x64,0xB5,0xF6,0x80) : QColor(0x19,0x76,0xD2,0x80);}

QColor ArrowToolButton::buttonFrameNormal() const  {return isDark ? QColor(0x61,0x61,0x61) : QColor(0xCB,0xCD,0xD3);}
QColor ArrowToolButton::buttonFramePressed() const {return isDark ? QColor(0x61,0x61,0x61) : QColor(0xCB,0xCD,0xD3);}
QColor ArrowToolButton::buttonFrameHovered() const {return isDark ? QColor(0x61,0x61,0x61) : QColor(0xCB,0xCD,0xD3);}
QColor ArrowToolButton::buttonFrameDisabled() const {return isDark ? QColor(0x26,0x27,0x29) : QColor(0xCB,0xCD,0xD3);}

QColor ArrowToolButton::buttonBackgroundNormal() const {return isDark ? QColor(0x61,0x61,0x61,0xB2) : QColor(0xFF,0xFF,0xFF,0xB2);}
QColor ArrowToolButton::buttonBackgroundPressed() const {return isDark ? QColor(0xFF,0xFF,0xFF,0x1F) : QColor(0x61,0x61,0x61,0x08);}
QColor ArrowToolButton::buttonBackgroundHovered() const {return isDark ? QColor(0xFF,0xFF,0xFF,0x08) : QColor(0x61,0x61,0x61,0x1F);}
QColor ArrowToolButton::buttonBackgroundDisabled() const {return isDark ? QColor(0x44,0x45,0x46) : QColor(0xF6,0xF6,0xF6);}
#else
QColor ArrowToolButton::buttonFrameFocused() const {return isDark ? QColor(0xFF,0xFF,0xFF) : QColor(0x21,0x21,0x21);}

QColor ArrowToolButton::buttonFrameNormal() const  {return isDark ? QColor(0xFF,0xFF,0xFF,0x18) : QColor(0xCB,0xCD,0xD3);}
QColor ArrowToolButton::buttonFramePressed() const {return isDark ? QColor(0xFF,0xFF,0xFF,0x12) : QColor(0xCB,0xCD,0xD3);}
QColor ArrowToolButton::buttonFrameHovered() const {return isDark ? QColor(0xFF,0xFF,0xFF,0x18) : QColor(0xCB,0xCD,0xD3);}
QColor ArrowToolButton::buttonFrameDisabled() const {return isDark ? QColor(0xFF,0xFF,0xFF,0x12) : QColor(0xCB,0xCD,0xD3);}

QColor ArrowToolButton::buttonBackgroundNormal() const {return isDark ? QColor(0xFF,0xFF,0xFF,0x0F) : QColor(0xFF,0xFF,0xFF,0xB2);}
QColor ArrowToolButton::buttonBackgroundPressed() const {return isDark ? QColor(0xFF,0xFF,0xFF,0x08) : QColor(0x61,0x61,0x61,0x08);}
QColor ArrowToolButton::buttonBackgroundHovered() const {return isDark ? QColor(0xFF,0xFF,0xFF,0x15) : QColor(0x61,0x61,0x61,0x1F);}
QColor ArrowToolButton::buttonBackgroundDisabled() const {return isDark ? QColor(0xFF,0xFF,0xFF,0x0B) : QColor(0xF6,0xF6,0xF6);}
#endif

} // namespace CUR
