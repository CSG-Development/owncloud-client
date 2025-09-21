#include "focusproxy.h"

FocusProxyStyle::FocusProxyStyle(QStyle* baseStyle)
    : QProxyStyle(baseStyle)
{

}

void FocusProxyStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_FrameFocusRect)
        return;

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}
