#pragma once

#include "stylebase.h"

namespace CUR {

class ProxyStyleToolButton : public ProxyStyleBase
{
public:
    explicit ProxyStyleToolButton(QStyle* baseStyle = nullptr);

    void drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    void drawControl(QStyle::ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const override;

protected:
    QColor buttonFrameFocused() const override;

    QColor buttonFrameNormal() const override;
    QColor buttonFramePressed() const override;
    QColor buttonFrameHovered() const override;
    QColor buttonFrameDisabled() const override;

    QColor buttonBackgroundNormal() const override;
    QColor buttonBackgroundPressed() const override;
    QColor buttonBackgroundHovered() const override;
    QColor buttonBackgroundDisabled() const override;

    QColor buttonBackgroundCheckedNormal() const override;
    QColor buttonBackgroundCheckedPressed() const override;
    QColor buttonBackgroundCheckedHovered() const override;

};

} // namespace CUR
