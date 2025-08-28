#include "curator_style.h"
#include "stylehelper.h"

#include <QPainterPath>
#include <QPushButton>
#include <QToolButton>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>

namespace {
constexpr qreal focusRound = 4;
constexpr qreal focusWidth = 1;
constexpr qreal frameRound = 3;
constexpr qreal frameWidth = 1;
}

namespace CUR {

QColor CuratorProxyStyle::buttonFrameFocused() const {return isDark ? QColor(0xFF,0xFF,0xFF) : QColor(0x21,0x21,0x21);}

QColor CuratorProxyStyle::buttonFrameNormal() const  {return isDark ? QColor() : QColor(0xCB,0xCD,0xD3);}
QColor CuratorProxyStyle::buttonFramePressed() const {return isDark ? QColor() : QColor(0xCB,0xCD,0xD3);}
QColor CuratorProxyStyle::buttonFrameHovered() const {return isDark ? QColor() : QColor(0xCB,0xCD,0xD3);}
QColor CuratorProxyStyle::buttonFrameDisabled() const {return isDark ? QColor() : QColor(0xCB,0xCD,0xD3);}

QColor CuratorProxyStyle::buttonBackgroundNormal() const {return isDark ? QColor() : QColor(0xFF,0xFF,0xFF,0xB2);}
QColor CuratorProxyStyle::buttonBackgroundPressed() const {return isDark ? QColor() : QColor(0x61,0x61,0x61,0x08);}
QColor CuratorProxyStyle::buttonBackgroundHovered() const {return isDark ? QColor() : QColor(0x61,0x61,0x61,0x1F);}
QColor CuratorProxyStyle::buttonBackgroundDisabled() const {return isDark ? QColor() : QColor(0xF6,0xF6,0xF6);}


CuratorProxyStyle::CuratorProxyStyle(QStyle* baseStyle)
    : ProxyStyleBase(baseStyle)
{
}

void CuratorProxyStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_FrameFocusRect) {
        if (const QStyleOptionFocusRect *fropt = qstyleoption_cast<const QStyleOptionFocusRect *>(option)) {
            if (!(fropt->state & State_KeyboardFocusChange))
                return;

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);

            painter->setPen(QPen(QColor(buttonFrameFocused()), focusWidth, Qt::SolidLine));
            painter->drawRoundedRect(option->rect, focusRound, focusRound);

            painter->restore();
        }
        return;
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void CuratorProxyStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == QStyle::CE_PushButtonBevel) {
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option))  {
            QRectF rect = btn->rect.adjusted(4, 4, -4, -4);

            QPainterPath path;
            path.addRoundedRect(rect, frameRound, frameRound);


            QColor bgColor;
            QColor frameColor;

            if (!(btn->state & QStyle::State_Enabled)) {
                bgColor = QColor(buttonBackgroundDisabled());
                frameColor = QColor(buttonFrameDisabled());
            }
            else if (btn->state & QStyle::State_Sunken) {
                // Pressed
                bgColor = QColor(buttonBackgroundPressed());
                frameColor = QColor(buttonFramePressed());
            }
            else if (btn->state & QStyle::State_MouseOver) {
                bgColor = QColor(buttonBackgroundHovered());
                frameColor = QColor(buttonFrameHovered());
            }
            else {
                bgColor = buttonBackgroundNormal();
                frameColor = buttonFrameNormal();
            }
            painter->save();

            painter->setRenderHint(QPainter::Antialiasing);

            painter->fillPath(path, bgColor);

            painter->setPen(QPen(frameColor, frameWidth, Qt::SolidLine));
            painter->drawPath(path);

            painter->restore();

            return;
        }
    }

    QProxyStyle::drawControl(element, option, painter, widget);
}

void CuratorProxyStyle::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole) const
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
