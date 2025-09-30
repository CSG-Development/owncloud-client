#include "ccheckbox.h"
#include "checkboxres.h"
#include "theme.h"
#include "focusproxy.h"

#include <QPainter>
#include <QStyleOptionButton>
#include <QFlags>
#include <QScreen>

namespace {
QPair<QColor,QColor> textColor = {QColor(0,0,0,0xDE), QColor(0xFF,0xFF,0xFF,0xDE)};
QPair<QColor,QColor> focusFrameColor = {QColor(0,0,0,0xDE), QColor(0xFF,0xFF,0xFF,0xDE)};
}

CCheckBox::CCheckBox(QWidget* parent)
    : QCheckBox(parent)
{
    CheckboxRes::init();
    setCursor(Qt::PointingHandCursor);
#ifdef Q_OS_MACOS
    setAttribute(Qt::WA_MacShowFocusRect, false);
#endif
}

CCheckBox::CCheckBox(const QString &text, QWidget *parent)
    : QCheckBox(text, parent)
{
    CheckboxRes::init();
    setCursor(Qt::PointingHandCursor);
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

void CCheckBox::setDarkTheme()
{
    isDark = CUR::Theme::instance()->isDarkTheme();
    update();
}

void CCheckBox::paintEvent(QPaintEvent* /*event*/)
{
    QStyleOptionButton so;
    initStyleOption(&so);

    QPainter painter(this);

    QRectF controlRect = rect().adjusted(frameWidth_, frameWidth_, -frameWidth_, -frameWidth_);
    QRectF contentRect = controlRect.adjusted(2, 2, -2, -2);

    painter.setRenderHint(QPainter::Antialiasing);

    int icon_offs = (contentRect.height() - iconSize().height()) / 2;

    const auto& icon = CheckboxRes::getChkIcon(so.state, isDark);
    icon.paint(&painter, QRect(QPoint(contentRect.x() + icon_offs, contentRect.y() + icon_offs), iconSize()));

    if (so.state.testFlag(QStyle::State_Enabled)) {
        if (so.state.testFlag(QStyle::State_HasFocus)) {
            painter.setPen(QPen(isDark ? focusFrameColor.second : focusFrameColor.first, frameWidth_, Qt::SolidLine));
            painter.drawPath(focusFrame(controlRect));
        }
    }

    QRectF textRect = contentRect;
    textRect.adjust(contentRect.x() + icon_offs * 2 + iconSize().width(), 1,  contentRect.x() + icon_offs, 0);

    int alignment = style()->visualAlignment(layoutDirection(), Qt::AlignLeft | Qt::AlignVCenter);

    if (!style()->styleHint(QStyle::SH_UnderlineShortcut, &so, this))
        alignment |= Qt::TextHideMnemonic;
    if (!text().isEmpty()) {
        alignment |= Qt::TextShowMnemonic;
    }

    painter.setPen(isDark ? textColor.second : textColor.first);
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
