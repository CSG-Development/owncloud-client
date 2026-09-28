#include "style_push_button.h"
#include "apppalette.h"
#include "stylehelper.h"

#include <QPainterPath>
#include <QPushButton>
#include <QToolButton>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>

namespace {
#ifdef Q_OS_MACOS
constexpr qreal focusRadius = 6;
constexpr qreal focusWidth = 3.5;
constexpr qreal frameRadius = 6;
constexpr qreal frameWidth = 0.5;
constexpr qreal focusFrameMargin = 5;
#else
constexpr qreal focusRadius = 5;
constexpr qreal focusWidth = 2;
constexpr qreal frameRadius = 4;
constexpr qreal frameWidth = 1;
constexpr qreal focusFrameMargin = 5;
#endif
constexpr int textPadding = 14;
}

namespace APP {

#ifdef Q_OS_MACOS
QColor ProxyStylePushButton::buttonFrameFocused() const {return AppPalette::color(ColorToken::PushButtonFocusRing, isDark);}

QColor ProxyStylePushButton::buttonFrameNormal() const  {return AppPalette::color(ColorToken::MacButtonFrameNormal, isDark);}
QColor ProxyStylePushButton::buttonFramePressed() const {return AppPalette::color(ColorToken::MacButtonFramePressed, isDark);}
QColor ProxyStylePushButton::buttonFrameHovered() const {return AppPalette::color(ColorToken::MacButtonFrameHovered, isDark);}
QColor ProxyStylePushButton::buttonFrameDisabled() const {return AppPalette::color(ColorToken::MacButtonFrameDisabled, isDark);}

QColor ProxyStylePushButton::buttonBackgroundNormal() const {return AppPalette::color(ColorToken::MacButtonBackgroundNormal, isDark);}
QColor ProxyStylePushButton::buttonBackgroundPressed() const {return AppPalette::color(ColorToken::MacButtonBackgroundPressed, isDark);}
QColor ProxyStylePushButton::buttonBackgroundHovered() const {return AppPalette::color(ColorToken::MacButtonBackgroundHovered, isDark);}
QColor ProxyStylePushButton::buttonBackgroundDisabled() const {return AppPalette::color(ColorToken::MacButtonBackgroundDisabled, isDark);}
#else
QColor ProxyStylePushButton::buttonFrameFocused() const {return AppPalette::color(ColorToken::ButtonFrameFocused, isDark);}

QColor ProxyStylePushButton::buttonFrameNormal() const  {return AppPalette::color(ColorToken::ButtonFrameNormal, isDark);}
QColor ProxyStylePushButton::buttonFramePressed() const {return AppPalette::color(ColorToken::ButtonFramePressed, isDark);}
QColor ProxyStylePushButton::buttonFrameHovered() const {return AppPalette::color(ColorToken::ButtonFrameHovered, isDark);}
QColor ProxyStylePushButton::buttonFrameDisabled() const {return AppPalette::color(ColorToken::ButtonFrameDisabled, isDark);}

QColor ProxyStylePushButton::buttonBackgroundNormal() const {return AppPalette::color(ColorToken::ButtonBackgroundNormal, isDark);}
QColor ProxyStylePushButton::buttonBackgroundPressed() const {return AppPalette::color(ColorToken::ButtonBackgroundPressed, isDark);}
QColor ProxyStylePushButton::buttonBackgroundHovered() const {return AppPalette::color(ColorToken::ButtonBackgroundHovered, isDark);}
QColor ProxyStylePushButton::buttonBackgroundDisabled() const {return AppPalette::color(ColorToken::ButtonBackgroundDisabled, isDark);}
#endif

ProxyStylePushButton::ProxyStylePushButton(QStyle* baseStyle)
    : ProxyStyleBase(baseStyle)
{
}

void ProxyStylePushButton::drawPrimitive(PrimitiveElement /*element*/, const QStyleOption* /*option*/, QPainter* /*painter*/, const QWidget* /*widget*/) const
{
    qt_noop();
    //QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void ProxyStylePushButton::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == QStyle::CE_PushButtonBevel) {
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option))  {
            QRectF rect = btn->rect.adjusted(::focusFrameMargin, ::focusFrameMargin, -::focusFrameMargin, -::focusFrameMargin);

            QPainterPath path;
            path.addRoundedRect(rect, ::frameRadius, ::frameRadius);

            QColor bgColor;
            QColor frameColor;
            bool isDisabled = (btn->state & QStyle::State_Enabled) == 0;
            bool isPressed = (btn->state & QStyle::State_Sunken) != 0;
            bool isHovered = (btn->state & QStyle::State_MouseOver) != 0;

            if (isDisabled) {
                bgColor = QColor(buttonBackgroundDisabled());
                frameColor = QColor(buttonFrameDisabled());
            }
            else if (isPressed) {
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
            painter->save();

            painter->setRenderHint(QPainter::Antialiasing);

            if (auto pButton = qobject_cast<const QPushButton*>(widget)) {
                if (pButton->hasFocus()) {
                    painter->setPen(QPen(QColor(buttonFrameFocused()), ::focusWidth, Qt::SolidLine));
                    painter->drawRoundedRect(
                        option->rect.adjusted(::focusWidth+1, ::focusWidth+1, -(::focusWidth+1), -(::focusWidth+1)),
                        ::focusRadius, ::focusRadius);
                }
            }

            painter->fillPath(path, bgColor);

            painter->setPen(QPen(frameColor, ::frameWidth, Qt::SolidLine));
            painter->drawPath(path);

            painter->restore();

            return;
        }
    }

    QProxyStyle::drawControl(element, option, painter, widget);
}

void ProxyStylePushButton::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole) const
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

QSize ProxyStylePushButton::sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize, const QWidget *w) const
{
    QSize sz = QProxyStyle::sizeFromContents(ct, opt, contentsSize, w);
    if (ct == CT_PushButton)
        sz.rwidth() += ::textPadding;
    return sz;
}

} // namespace APP
