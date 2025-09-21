#include "loginpushbutton.h"

#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionButton>

namespace {
constexpr int iconSize = 18;
constexpr int radius = 24;
constexpr int iconPadding = 8;

QPair<QColor,QColor> backNormalColor   = {QColor("#1976D2"), QColor("#64B5F6")};
QPair<QColor,QColor> backDisabledColor = {QColor("#EEEEEE"), QColor("#616161")};
QPair<QColor,QColor> textNormalColor   = {QColor("#FFFFFF"), QColor("#212121")};
QPair<QColor,QColor> textDisabledColor = {QColor("#9E9E9E"), QColor("#9E9E9E")};

}


LoginPushButton::LoginPushButton(QWidget *parent)
    : QPushButton(parent)
{
}

void LoginPushButton::setDarkTheme(bool dark)
{
    isDark = dark;
    update();
}

void LoginPushButton::paintEvent(QPaintEvent* /*event*/)
{
    QStyleOptionButton option;
    initStyleOption(&option);

    QPainterPath pp;
    pp.addRoundedRect(rect(), radius, radius);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    bool isEnabled = option.state.testFlag(QStyle::State_Enabled);

    painter.fillPath(pp, isEnabled ? (isDark ? backNormalColor.second : backNormalColor.first) :
                                     (isDark ? backDisabledColor.second : backDisabledColor.first));


    if (!icon_.isNull()) {
        QRect iconRect;
        int iconY = (rect().height() - ::iconSize) / 2;

        if (side_ == IconSidePosition::Left)
            iconRect = QRect(rect().left() + iconPadding, iconY, ::iconSize, ::iconSize);
        else
            iconRect = QRect(rect().right() - iconPadding - ::iconSize, iconY, ::iconSize, ::iconSize);

        icon_.paint(&painter, iconRect);
    }

    int offset = iconPadding * 2 + ::iconSize;
    QRect textRect = rect();
    if (side_ == IconSidePosition::Left) {
        textRect.adjust(offset, 0, -iconPadding, 0);
    }
    else {
        textRect.adjust(offset, 0, -iconPadding, 0);
    }

    int alignment = style()->visualAlignment(layoutDirection(), Qt::AlignLeft | Qt::AlignVCenter);
    if (!style()->styleHint(QStyle::SH_UnderlineShortcut, &option, this))
        alignment |= Qt::TextHideMnemonic;
    if (!text().isEmpty()) {
        alignment |= Qt::TextShowMnemonic;
    }

    QColor textColor = isEnabled ? (isDark ? textNormalColor.second : textNormalColor.first) :
                                   (isDark ? textDisabledColor.second : textDisabledColor.first);
    painter.setPen(textColor);
    painter.drawText(textRect, alignment, text());
}
