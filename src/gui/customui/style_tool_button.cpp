#include "style_tool_button.h"
#include "apppalette.h"
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

namespace APP {

QColor ProxyStyleToolButton::buttonFrameFocused() const {return AppPalette::color(ColorToken::ToolButtonFocusRing, isDark);}

QColor ProxyStyleToolButton::buttonFrameNormal() const  {return {};}
QColor ProxyStyleToolButton::buttonFramePressed() const {return {};}
QColor ProxyStyleToolButton::buttonFrameHovered() const {return {};}
QColor ProxyStyleToolButton::buttonFrameDisabled() const {return {};}

QColor ProxyStyleToolButton::buttonBackgroundNormal() const {return AppPalette::color(ColorToken::ToolButtonBackgroundNormal, isDark);}
QColor ProxyStyleToolButton::buttonBackgroundPressed() const {return AppPalette::color(ColorToken::ToolButtonBackgroundPressed, isDark);}
QColor ProxyStyleToolButton::buttonBackgroundHovered() const {return AppPalette::color(ColorToken::ToolButtonBackgroundHovered, isDark);}
QColor ProxyStyleToolButton::buttonBackgroundDisabled() const {return AppPalette::color(ColorToken::ToolButtonBackgroundDisabled, isDark);}

QColor ProxyStyleToolButton::buttonBackgroundCheckedNormal() const {return AppPalette::color(ColorToken::ToolButtonCheckedBackground, isDark);}
QColor ProxyStyleToolButton::buttonBackgroundCheckedPressed() const {return AppPalette::color(ColorToken::ToolButtonCheckedBackground, isDark);}
QColor ProxyStyleToolButton::buttonBackgroundCheckedHovered() const {return AppPalette::color(ColorToken::ToolButtonCheckedHoverBackground, isDark);}

ProxyStyleToolButton::ProxyStyleToolButton(QStyle* baseStyle)
    : ProxyStyleBase(baseStyle)
{
}

void ProxyStyleToolButton::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
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

        QColor bgColor;
        if (auto btn = qobject_cast<const QToolButton*>(widget)) {
            if (btn->isChecked()) {
                if (!(option->state & QStyle::State_Enabled)) {
                    bgColor = buttonBackgroundDisabled();
                }
                else if (option->state & QStyle::State_Sunken) {
                    // Pressed
                    bgColor = buttonBackgroundCheckedPressed();
                }
                else if (option->state & QStyle::State_MouseOver) {
                    bgColor = buttonBackgroundCheckedHovered();
                }
                else {
                    bgColor = buttonBackgroundCheckedNormal();
                }
            }
            else {
                if (!(option->state & QStyle::State_Enabled)) {
                    bgColor = buttonBackgroundDisabled();
                }
                else if (option->state & QStyle::State_Sunken) {
                    // Pressed
                    bgColor = buttonBackgroundPressed();
                }
                else if (option->state & QStyle::State_MouseOver) {
                    bgColor = buttonBackgroundHovered();
                }
                else {
                    bgColor = buttonBackgroundNormal();
                }
            }

            painter->fillRect(option->rect, bgColor);
            return;
        }
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void ProxyStyleToolButton::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == CE_ToolButtonLabel) {
        if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
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
    QProxyStyle::drawControl(element, option, painter, widget);

}

void ProxyStyleToolButton::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole) const
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

} // namespace APP
