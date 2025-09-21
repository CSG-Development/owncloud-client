#include "cradiobutton.h"
#include "radiobuttonres.h"

#include <QPainter>
#include <QStyleOptionButton>
#include <QFlags>

CRadioButton::CRadioButton(QWidget* parent)
    : QRadioButton(parent)
{
    RadiobuttonRes::init();
    setCursor(Qt::PointingHandCursor);
}

CRadioButton::CRadioButton(const QString &text, QWidget *parent)
    : QRadioButton(text, parent)
{
    RadiobuttonRes::init();
    setCursor(Qt::PointingHandCursor);
}

QSize CRadioButton::minimumSizeHint() const
{
    return {qMin(sizeHint().width(), 34), 34};
}

QSize CRadioButton::sizeHint() const
{
    ensurePolished();

    auto szHint = QRadioButton::sizeHint();
    szHint.setWidth(qMax(34, szHint.width() + iconSize_.width() + 2));
    return szHint;
}

void CRadioButton::paintEvent(QPaintEvent */*event*/)
{
    QStyleOptionButton so;
    initStyleOption(&so);

    QPainter painter(this);

    QRectF controlRect = rect().adjusted(frameWidth(), frameWidth(), -frameWidth(), -frameWidth());
    QRectF contentRect = controlRect.adjusted(2, 2, -2, -2);

    painter.setRenderHint(QPainter::Antialiasing);

    int icon_offs = (contentRect.height() - iconSize().height()) / 2;

    const auto& icon = RadiobuttonRes::getRbIcon(so.state);
    icon.paint(&painter, QRect(QPoint(contentRect.x() + icon_offs, contentRect.y() + icon_offs), iconSize()));

    if (so.state.testFlag(QStyle::State_Enabled)) {
        if (so.state.testFlag(QStyle::State_HasFocus)) {
            painter.setPen(QPen(frameColor(), frameWidth(), Qt::SolidLine));
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

    painter.setPen(textColor());
    painter.drawText(textRect, alignment, text());
}

bool CRadioButton::hitButton(const QPoint &pos) const
{
    /**
     * The mouse cursor changes when hovering over the control area, but the state is not switched by clicking.
     * Therefore, we use the entire control area to avoid user confusion.
     * The reason for this is that we are not using the standard style and function
     * subElementRect(QStyle::SE_CheckBoxClickRect, ...) calculates the click area incorrectly.
     */

    return rect().contains(pos);
}

QPainterPath CRadioButton::focusFrame(const QRectF &r)
{
    QPainterPath p;
    p.addRoundedRect(r, frameRadius(), frameRadius());
    return p;
}
