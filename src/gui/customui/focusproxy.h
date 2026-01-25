#pragma once

#include <QProxyStyle>

class FocusProxyStyle: public QProxyStyle
{
public:
    explicit FocusProxyStyle(QObject* parent = nullptr)
        : QProxyStyle()
    {
        setParent(parent);
    }
    explicit FocusProxyStyle(QStyle* baseStyle = nullptr);

    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
};
