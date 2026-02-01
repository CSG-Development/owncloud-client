#pragma once

#include <QProxyStyle>
#include <QDebug>

namespace APP {

class ProxyStyleBase: public QProxyStyle
{
public:
    explicit ProxyStyleBase(QStyle* style = nullptr);

    void setDarkMode(bool dark) {
        isDark = dark;
    }


protected:
    virtual QColor buttonFrameFocused() const;

    virtual QColor buttonFrameNormal() const;
    virtual QColor buttonFramePressed() const;
    virtual QColor buttonFrameHovered() const;
    virtual QColor buttonFrameDisabled() const;

    virtual QColor buttonBackgroundNormal() const;
    virtual QColor buttonBackgroundPressed() const;
    virtual QColor buttonBackgroundHovered() const;
    virtual QColor buttonBackgroundDisabled() const;

    virtual QColor buttonBackgroundCheckedNormal() const;
    virtual QColor buttonBackgroundCheckedPressed() const;
    virtual QColor buttonBackgroundCheckedHovered() const;

    bool isDark = false;
};

} // namespace APP
