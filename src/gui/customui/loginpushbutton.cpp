#include "loginpushbutton.h"
#include "theme.h"

#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionButton>

namespace {
constexpr int iconSize = 19;
constexpr int radius = 24;
constexpr int iconPadding = 17;

std::pair<QColor,QColor> backNormalColor   = {QColor("#1976D2"), QColor("#64B5F6")};
std::pair<QColor,QColor> backHoverColor   = {QColor("#1E88E5"), QColor("#90CAF9")};
std::pair<QColor,QColor> backPressedColor   = {QColor("#145CA4"), QColor("#359EF3")};
std::pair<QColor,QColor> backDisabledColor = {QColor("#EEEEEE"), QColor("#616161")};
std::pair<QColor,QColor> textNormalColor   = {QColor("#FFFFFF"), QColor("#212121")};
std::pair<QColor,QColor> textDisabledColor = {QColor("#9E9E9E"), QColor("#9E9E9E")};

}

LoginPushButton::LoginPushButton(QWidget *parent)
    : QPushButton(parent)
{
    themeNotifier = darkTheme_.addNotifier([this] {
        update();
    });
    darkTheme_.setValue(APP::Theme::instance()->isDarkTheme());
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
    bool isHovered = option.state & QStyle::State_MouseOver;
    bool isPressed = option.state & QStyle::State_Sunken;

    QColor currentColor = darkTheme_.value() ? backNormalColor.second : backNormalColor.first;
    if (isEnabled) {
        if (isPressed) {
            currentColor = darkTheme_.value() ? backPressedColor.second : backPressedColor.first;
        } else if (isHovered) {
            currentColor = darkTheme_.value() ? backHoverColor.second : backHoverColor.first;
        }
    } else {
        currentColor = darkTheme_.value() ? backDisabledColor.second : backDisabledColor.first;
    }

    painter.fillPath(pp, currentColor);

    if (!icon_.isNull()) {
        QRect iconRect;
        int iconY = (rect().height() - ::iconSize) / 2;

        if (side_ == IconSidePosition::Left)
            iconRect = QRect(rect().left() + iconPadding, iconY, ::iconSize, ::iconSize);
        else
            iconRect = QRect(rect().right() - iconPadding - ::iconSize, iconY, ::iconSize, ::iconSize);

        icon_.paint(&painter, iconRect);
    }

    int offset = iconPadding + ::iconSize + 8;
    QRect textRect = rect();
    if (side_ == IconSidePosition::Left) {
        textRect.adjust(offset, 0, -iconPadding, -2);
    }
    else {
        textRect.adjust(iconPadding, 0, -offset-2, -2);
    }

    int alignment = style()->visualAlignment(layoutDirection(), Qt::AlignLeft | Qt::AlignVCenter);
    if (!style()->styleHint(QStyle::SH_UnderlineShortcut, &option, this))
        alignment |= Qt::TextHideMnemonic;
    if (!text().isEmpty()) {
        alignment |= Qt::TextShowMnemonic;
    }

    QColor textColor = isEnabled ? (darkTheme_.value() ? textNormalColor.second : textNormalColor.first) :
                                   (darkTheme_.value() ? textDisabledColor.second : textDisabledColor.first);
    painter.setPen(textColor);
    QFont f = font();
    f.setBold(true);
    f.setPixelSize(14);
    painter.setFont(f);
    painter.drawText(textRect, alignment, text());
}
