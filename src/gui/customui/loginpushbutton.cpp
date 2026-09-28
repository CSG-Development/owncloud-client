#include "loginpushbutton.h"
#include "apppalette.h"
#include "theme.h"

#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionButton>

namespace {
constexpr int iconSize = 19;
constexpr int radius = 24;
constexpr int iconPadding = 17;
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

    const bool isDark = darkTheme_.value();
    QColor currentColor = APP::AppPalette::color(APP::ColorToken::Primary, isDark);
    if (isEnabled) {
        if (isPressed) {
            currentColor = APP::AppPalette::color(APP::ColorToken::PrimaryPressed, isDark);
        } else if (isHovered) {
            currentColor = APP::AppPalette::color(APP::ColorToken::PrimaryHover, isDark);
        }
    } else {
        currentColor = APP::AppPalette::color(APP::ColorToken::PrimaryDisabled, isDark);
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

    QColor textColor = APP::AppPalette::color(isEnabled ? APP::ColorToken::OnPrimary : APP::ColorToken::OnPrimaryDisabled, isDark);
    painter.setPen(textColor);
    QFont f = font();
    f.setBold(true);
    f.setPixelSize(14);
    painter.setFont(f);
    painter.drawText(textRect, alignment, text());
}
