#include "menu_toolbutton.h"
#include "apppalette.h"
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


QColor MenuToolButton::buttonFrameFocused() const {return AppPalette::color(ColorToken::ToolButtonFocusRing, isDark);}

QColor MenuToolButton::buttonBackgroundNormal() const {return AppPalette::color(ColorToken::ToolButtonBackgroundNormal, isDark);}
QColor MenuToolButton::buttonBackgroundPressed() const {return AppPalette::color(ColorToken::ToolButtonBackgroundPressed, isDark);}
QColor MenuToolButton::buttonBackgroundHovered() const {return AppPalette::color(ColorToken::ToolButtonBackgroundHovered, isDark);}
QColor MenuToolButton::buttonBackgroundDisabled() const {return AppPalette::color(ColorToken::ToolButtonBackgroundDisabled, isDark);}

QColor MenuToolButton::buttonBackgroundCheckedNormal() const {return AppPalette::color(ColorToken::TabCheckedBackground, isDark);}
QColor MenuToolButton::buttonBackgroundCheckedPressed() const {return AppPalette::color(ColorToken::TabCheckedBackground, isDark);}
QColor MenuToolButton::buttonBackgroundCheckedHovered() const {return AppPalette::color(ColorToken::TabCheckedHoverBackground, isDark);}

} // namespace APP
