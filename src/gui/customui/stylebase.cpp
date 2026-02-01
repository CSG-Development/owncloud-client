#include "stylebase.h"

namespace APP {

ProxyStyleBase::ProxyStyleBase(QStyle* style)
    : QProxyStyle(style)
{
}

QColor ProxyStyleBase::buttonFrameFocused() const {return QColor();}

QColor ProxyStyleBase::buttonFrameNormal() const  {return QColor();}
QColor ProxyStyleBase::buttonFramePressed() const {return QColor();}
QColor ProxyStyleBase::buttonFrameHovered() const {return QColor();}
QColor ProxyStyleBase::buttonFrameDisabled() const {return QColor();}

QColor ProxyStyleBase::buttonBackgroundNormal() const {return QColor();}
QColor ProxyStyleBase::buttonBackgroundPressed() const {return QColor();}
QColor ProxyStyleBase::buttonBackgroundHovered() const {return QColor();}
QColor ProxyStyleBase::buttonBackgroundDisabled() const {return QColor();}

QColor ProxyStyleBase::buttonBackgroundCheckedNormal() const {return QColor();}
QColor ProxyStyleBase::buttonBackgroundCheckedPressed() const {return QColor();}
QColor ProxyStyleBase::buttonBackgroundCheckedHovered() const {return QColor();}

} // namespace APP
