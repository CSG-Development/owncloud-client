#include "ccheckbox.h"
#include "checkboxres.h"
#include "theme.h"
#include "focusproxy.h"

#include <QPainter>
#include <QStyleOptionButton>
#include <QStyle>
#include <QFlags>
#include <QScreen>

namespace {
std::pair<QColor,QColor> textColor =       {QColor(0,0,0,0xDE), QColor(0xFF,0xFF,0xFF,0xDE)};
std::pair<QColor,QColor> focusFrameColor = {QColor(0,0,0,0xDE), QColor(0xFF,0xFF,0xFF,0xDE)};
const QSize icon_size = {20, 20};
}

CCheckBox::CCheckBox(QWidget* parent)
    : QCheckBox(parent)
{
    CheckboxRes::init();
    setCursor(Qt::PointingHandCursor);
#ifdef Q_OS_MACOS
    setAttribute(Qt::WA_MacShowFocusRect, false);
#endif
    themeNotifier = darkTheme_.addNotifier([this] {
        update();
    });
    darkTheme_.setValue(APP::Theme::instance()->isDarkTheme());
}

CCheckBox::CCheckBox(const QString &text, QWidget *parent)
    : QCheckBox(text, parent)
{
    CheckboxRes::init();
    setCursor(Qt::PointingHandCursor);
    themeNotifier = darkTheme_.addNotifier([this] {
        update();
    });
    darkTheme_.setValue(APP::Theme::instance()->isDarkTheme());
}

QSize CCheckBox::minimumSizeHint() const
{
    int w = qMax(sizeHint().width(), 34);
    return {w, 34};
}

QSize CCheckBox::sizeHint() const
{
    ensurePolished();

    auto szHint = QCheckBox::sizeHint();
    szHint.setWidth(qMax(34, szHint.width() + iconSize_.width() + 2));
    return szHint;
}

void CCheckBox::paintEvent(QPaintEvent* /*event*/)
{
    QStyleOptionButton so;
    initStyleOption(&so);

    QPainter painter(this);

    QRectF controlRect = rect().adjusted(frameWidth_, frameWidth_, -frameWidth_, -frameWidth_);
    QRectF contentRect = controlRect.adjusted(2, 2, -2, -2);

    painter.setRenderHint(QPainter::Antialiasing);

    int icon_offs = (contentRect.height() - icon_size.height()) / 2;

    const bool checked  = so.state & QStyle::State_On;
    const bool hovered  = so.state & QStyle::State_MouseOver;
    const bool pressed  = so.state & QStyle::State_Sunken;
    const bool disabled = !(so.state & QStyle::State_Enabled);
    const bool dark     = darkTheme_.value();

    const auto& icon = pickIcon(checked, hovered, pressed, disabled, dark);
    icon.paint(&painter, QRect(QPoint(contentRect.x() + icon_offs, contentRect.y() + icon_offs), icon_size));

    if (so.state.testFlag(QStyle::State_Enabled)) {
        if (so.state.testFlag(QStyle::State_HasFocus)) {
            painter.setPen(QPen(darkTheme_.value() ? focusFrameColor.second : focusFrameColor.first, frameWidth_, Qt::SolidLine));
            painter.drawPath(focusFrame(controlRect));
        }
    }

    QRectF textRect = contentRect;
    textRect.adjust(contentRect.x() + icon_offs * 2 + icon_size.width(), 1,  contentRect.x() + icon_offs, 0);

    int alignment = style()->visualAlignment(layoutDirection(), Qt::AlignLeft | Qt::AlignVCenter);

    if (!style()->styleHint(QStyle::SH_UnderlineShortcut, &so, this))
        alignment |= Qt::TextHideMnemonic;
    if (!text().isEmpty()) {
        alignment |= Qt::TextShowMnemonic;
    }

    painter.setPen(darkTheme_.value() ? textColor.second : textColor.first);
    painter.drawText(textRect, alignment, text());
}

bool CCheckBox::hitButton(const QPoint &pos) const
{
    /**
     * The mouse cursor changes when hovering over the control area, but the state is not switched by clicking.
     * Therefore, we use the entire control area to avoid user confusion.
     * The reason for this is that we are not using the standard style and function
     * subElementRect(QStyle::SE_CheckBoxClickRect, ...) calculates the click area incorrectly.
     */

    return rect().contains(pos);
}

QPainterPath CCheckBox::focusFrame(const QRectF& r)
{
    QPainterPath p;
    p.addRoundedRect(r, frameRadius_, frameRadius_);
    return p;
}

QIcon CCheckBox::pickIcon(bool checked, bool hovered, bool pressed, bool disabled, bool dark)
{
    static const auto _ = [] {
        Q_INIT_RESOURCE(customui_res);
        return true;
    }();

    // state: 0=normal, 1=hover, 2=pressed, 3=disabled
#ifdef Q_OS_MACOS
    static const QIcon icons[2][2][4] = {
    { // light
        { // unchecked
            QIcon(QStringLiteral(":/res/checkbox_mac/light/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/light/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/light/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/light/disabled/unchecked.svg")),
        },
        { // checked
            QIcon(QStringLiteral(":/res/checkbox_mac/light/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/light/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/light/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/light/disabled/checked.svg")),
        },
    },
    { // dark
        { // unchecked dark
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/disabled/unchecked.svg")),
        },
        { // checked dark
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_mac/dark/disabled/checked.svg")),
        },
        },
    };
#else
    static const QIcon icons[2][2][4] = {
    { // light
        { // unchecked
            QIcon(QStringLiteral(":/res/checkbox_win/light/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/light/hover/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/light/pressed/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/light/disabled/unchecked.svg")),
        },
        { // checked
            QIcon(QStringLiteral(":/res/checkbox_win/light/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/light/hover/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/light/pressed/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/light/disabled/checked.svg")),
        },
    },
    { // dark
        { /* unchecked dark ... */
            QIcon(QStringLiteral(":/res/checkbox_win/dark/normal/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/dark/hover/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/dark/pressed/unchecked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/dark/disabled/unchecked.svg")),
        },
        { /* checked dark ... */
            QIcon(QStringLiteral(":/res/checkbox_win/dark/normal/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/dark/hover/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/dark/pressed/checked.svg")),
            QIcon(QStringLiteral(":/res/checkbox_win/dark/disabled/checked.svg")),
        },
        },
    };
#endif
    const int state = disabled ? 3 : pressed ? 2 : hovered ? 1 : 0;
    return icons[dark ? 1 : 0][checked ? 1 : 0][state];
}
