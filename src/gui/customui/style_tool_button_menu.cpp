#include "style_tool_button_menu.h"
#include "stylehelper.h"

#include <QPainterPath>
#include <QPushButton>
#include <QToolButton>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>

namespace {
constexpr qreal frameRound = 3.5;
QSize iconSize = {40, 40};
}

namespace CUR {

QColor ProxyStyleToolButtonMenu::buttonFrameFocused() const {return isDark ? QColor(0x19,0x76,0xD2,0x80) : QColor(0x19,0x76,0xD2,0x80);}

QColor ProxyStyleToolButtonMenu::buttonFrameNormal() const  {return {};}
QColor ProxyStyleToolButtonMenu::buttonFramePressed() const {return {};}
QColor ProxyStyleToolButtonMenu::buttonFrameHovered() const {return {};}
QColor ProxyStyleToolButtonMenu::buttonFrameDisabled() const {return {};}

QColor ProxyStyleToolButtonMenu::buttonBackgroundNormal() const {return isDark ? QColor(0,0,0,0) : QColor(0,0,0,0);}
QColor ProxyStyleToolButtonMenu::buttonBackgroundPressed() const {return isDark ? QColor(0x4E,0x50,0x53) : QColor(0xF5,0xF5,0xF7);}
QColor ProxyStyleToolButtonMenu::buttonBackgroundHovered() const {return isDark ? QColor(0xFF,0xFF,0xFF,0x1A) : QColor(0xE6,0xE3,0xE6);}
QColor ProxyStyleToolButtonMenu::buttonBackgroundDisabled() const {return isDark ? QColor(0,0,0,0) : QColor(0,0,0,0);}

QColor ProxyStyleToolButtonMenu::buttonBackgroundCheckedNormal() const {return isDark ? QColor(0x90,0xCA,0xF9,0x1F) : QColor(0x1E,0x88,0xE5,0x1F);}
QColor ProxyStyleToolButtonMenu::buttonBackgroundCheckedPressed() const {return isDark ? QColor(0x90,0xCA,0xF9,0x1F) : QColor(0x1E,0x88,0xE5,0x1F);}
QColor ProxyStyleToolButtonMenu::buttonBackgroundCheckedHovered() const {return isDark ? QColor(0x64,0xB5,0xF6,0x3D) : QColor(0x19,0x76,0xD2,0x33);}

ProxyStyleToolButtonMenu::ProxyStyleToolButtonMenu(QStyle* baseStyle)
    : ProxyStyleBase(baseStyle)
{
}

void ProxyStyleToolButtonMenu::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    qDebug() << element << option->state;

    if (element == QStyle::PE_FrameFocusRect) {
        if (const QStyleOptionFocusRect *fropt = qstyleoption_cast<const QStyleOptionFocusRect *>(option)) {
            if (!(fropt->state & State_KeyboardFocusChange))
                return;

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setPen(QPen(buttonFrameFocused(), frameRound, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
            painter->drawRect(option->rect.adjusted(-1, -1, 1, 1));

            painter->restore();
        }
        return;
    }

    if (element == QStyle::PE_PanelButtonTool) {
        return;
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void ProxyStyleToolButtonMenu::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == CE_ToolButtonLabel) {
        if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {

            QColor bgColor;
            bool isHovered = (option->state & QStyle::State_MouseOver) != 0;
            bool isPressed = (option->state & QStyle::State_Sunken) != 0;
            bool isDisabled = (option->state & QStyle::State_Enabled) == 0;
            if (auto btn = qobject_cast<const QToolButton*>(widget)) {
                if (btn->isChecked()) {
                    if (isDisabled) bgColor = buttonBackgroundDisabled();
                    else if (isPressed) bgColor = buttonBackgroundCheckedPressed();
                    else if (isHovered) bgColor = buttonBackgroundCheckedHovered();
                    else bgColor = buttonBackgroundCheckedNormal();
                }
                else {
                    if (isDisabled) bgColor = buttonBackgroundDisabled();
                    else if (isPressed) bgColor = buttonBackgroundPressed();
                    else if (isHovered) bgColor = buttonBackgroundHovered();
                    else bgColor = buttonBackgroundNormal();
                }

                painter->fillRect(option->rect, bgColor);
            }

            QIcon::State state = tb->state & State_On ? QIcon::On : QIcon::Off;
            QIcon::Mode mode;
            if (!(tb->state & State_Enabled))
                mode = QIcon::Disabled;
            else if ((tb->state & State_MouseOver) && (tb->state & State_AutoRaise))
                mode = QIcon::Active;
            else
                mode = QIcon::Normal;

            QRect pix_r = tb->rect;
            QRect text_r = tb->rect;

            pix_r.setHeight(iconSize.height() + 4);
            text_r.adjust(0, pix_r.height() - 14, 0, -1);

            auto pm = tb->icon.pixmap(tb->rect.size().boundedTo(tb->iconSize), painter->device()->devicePixelRatio(), mode, state);

            int alignment = Qt::TextShowMnemonic;
            alignment |= Qt::AlignCenter;

            QFontMetrics fontMetrics = painter->fontMetrics();
            const QString elidedLine = fontMetrics.elidedText(tb->text, Qt::ElideMiddle, text_r.width());

            proxy()->drawItemPixmap(painter, pix_r, Qt::AlignCenter, pm);
            proxy()->drawItemText(painter, QStyle::visualRect(tb->direction, tb->rect, text_r), alignment, tb->palette, tb->state & State_Enabled, elidedLine);

            return;
        }
    }
    //QProxyStyle::drawControl(element, option, painter, widget);

}

void ProxyStyleToolButtonMenu::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    Q_UNUSED(enabled)

    if (text.isEmpty())
        return;

    QPen savedPen = painter->pen();
    if (textRole != QPalette::NoRole)
        painter->setPen(QPen(pal.brush(textRole), savedPen.widthF()));
    painter->drawText(rect, flags, text);
    painter->setPen(savedPen);
}

} // namespace CUR
