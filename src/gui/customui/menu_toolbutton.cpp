#include "menu_toolbutton.h"
#include "theme.h"

#include <QPainter>
#include <QMouseEvent>
#include <QStyle>

namespace {
constexpr qreal focusRound = 4;
constexpr qreal focusWidth = 1;
constexpr qreal frameRound = 3;
constexpr qreal frameWidth = 1;
QSize toolbarIconSize = {40,40};
}

namespace APP {


MenuToolButton::MenuToolButton(QWidget *parent)
    : QToolButton(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMinimumHeight(90);
}

void MenuToolButton::paintEvent(QPaintEvent* /*event*/)
{
    isDark = Theme::instance()->isDarkTheme();

    QPainter painter(this);

    QColor bgColor;
    if (isChecked()) {
        if (!isEnabled())   {bgColor = buttonBackgroundDisabled();}
        else if (isPressed) {bgColor = buttonBackgroundCheckedPressed();}
        else if (isHovered) {bgColor = buttonBackgroundCheckedHovered(); qDebug() << "hover" << bgColor.name(QColor::HexArgb);}
        else                {bgColor = buttonBackgroundCheckedNormal();}
    }
    else {
        if (!isEnabled())   {bgColor = buttonBackgroundDisabled();}
        else if (isPressed) {bgColor = buttonBackgroundPressed();}
        else if (isHovered) {bgColor = buttonBackgroundHovered();}
        else                {bgColor = buttonBackgroundNormal();}
    }

    painter.fillRect(rect(), bgColor);

    QRect pix_r = rect();
    QRect text_r = pix_r;

    pix_r.setHeight(::toolbarIconSize.height() + 4);
    text_r.adjust(0, pix_r.height() - 14, 0, -1);

    const auto ic = icon();
    if (!ic.isNull()) {
        ic.paint(&painter, pix_r, Qt::AlignCenter);
    }

    int alignment = Qt::TextShowMnemonic;
    alignment |= Qt::AlignCenter;

    QFontMetrics fontMetrics = painter.fontMetrics();
    const QString elidedLine = fontMetrics.elidedText(text(), Qt::ElideMiddle, text_r.width());

    style()->drawItemText(&painter, QStyle::visualRect(layoutDirection(), rect(), text_r), alignment, palette(), isEnabled(), elidedLine);
}

void MenuToolButton::enterEvent(QEnterEvent *event)
{
    isHovered = true;
    QToolButton::enterEvent(event);
}

void MenuToolButton::leaveEvent(QEvent *event)
{
    isHovered = false;
    QToolButton::leaveEvent(event);
}

void MenuToolButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isPressed = true;
    }
    QToolButton::mousePressEvent(event);
}

void MenuToolButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isPressed = false;
    }
    QToolButton::mouseReleaseEvent(event);
}


QColor MenuToolButton::buttonFrameFocused() const {return isDark ? QColor(0x19,0x76,0xD2,0x80) : QColor(0x19,0x76,0xD2,0x80);}

QColor MenuToolButton::buttonBackgroundNormal() const {return isDark ? QColor(0,0,0,0) : QColor(0,0,0,0);}
QColor MenuToolButton::buttonBackgroundPressed() const {return isDark ? QColor(0x4E,0x50,0x53) : QColor(0xF5,0xF5,0xF7);}
QColor MenuToolButton::buttonBackgroundHovered() const {return isDark ? QColor(0xFF,0xFF,0xFF,0x1A) : QColor(0xE6,0xE3,0xE6);}
QColor MenuToolButton::buttonBackgroundDisabled() const {return isDark ? QColor(0,0,0,0) : QColor(0,0,0,0);}

QColor MenuToolButton::buttonBackgroundCheckedNormal() const {return isDark ? QColor(0x90,0xCA,0xF9,0x1F) : QColor(0x1E,0x88,0xE5,0x1F);}
QColor MenuToolButton::buttonBackgroundCheckedPressed() const {return isDark ? QColor(0x90,0xCA,0xF9,0x1F) : QColor(0x1E,0x88,0xE5,0x1F);}
QColor MenuToolButton::buttonBackgroundCheckedHovered() const {return isDark ? QColor(0x64,0xB5,0xF6,0x3D) : QColor(0x19,0x76,0xD2,0x33);}

} // namespace APP
