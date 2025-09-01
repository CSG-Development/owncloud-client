#include "tool_button_dots.h"
#include "stylehelper.h"

#include <QPainterPath>
#include <QPushButton>
#include <QToolButton>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>

QColor tbFrameFocused(bool isDark) {return isDark ? QColor(0xFF,0xFF,0xFF) : QColor(0x21,0x21,0x21);}

QColor tbFrameNormal(bool isDark) {return isDark ? QColor(0xFF,0xFF,0xFF,0x18) : QColor(0xCB,0xCD,0xD3);}
QColor tbFramePressed(bool isDark) {return isDark ? QColor(0xFF,0xFF,0xFF,0x12) : QColor(0xCB,0xCD,0xD3);}
QColor tbFrameHovered(bool isDark) {return isDark ? QColor(0xFF,0xFF,0xFF,0x18) : QColor(0xCB,0xCD,0xD3);}
QColor tbFrameDisabled(bool isDark) {return isDark ? QColor(0xFF,0xFF,0xFF,0x12) : QColor(0xCB,0xCD,0xD3);}

QColor tbBackgroundNormal(bool isDark) {return isDark ? QColor(0xFF,0xFF,0xFF,0x0F) : QColor(0xFF,0xFF,0xFF,0xB2);}
QColor tbBackgroundPressed(bool isDark) {return isDark ? QColor(0xFF,0xFF,0xFF,0x08) : QColor(0x61,0x61,0x61,0x08);}
QColor tbBackgroundHovered(bool isDark) {return isDark ? QColor(0xFF,0xFF,0xFF,0x15) : QColor(0x61,0x61,0x61,0x1F);}
QColor tbBackgroundDisabled(bool isDark) {return isDark ? QColor(0,0,0,0) : QColor(0xF6,0xF6,0xF6);}

constexpr int frameRound = 3;

namespace CUR {

void ToolButtonDots::drawButton(QStyleOptionToolButton *opt, QPainter *painter, bool isDark)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    QColor bgColor;
    QColor frameColor;

    if (!(opt->state & QStyle::State_Enabled)) {
        bgColor = tbBackgroundDisabled(isDark);
        frameColor = tbFrameDisabled(isDark);
    }
    else if (opt->state & QStyle::State_Sunken) {
        // Pressed
        bgColor = tbBackgroundPressed(isDark);
        frameColor = tbFramePressed(isDark);
    }
    else if (opt->state & QStyle::State_MouseOver) {
        bgColor = tbBackgroundHovered(isDark);
        frameColor = tbFrameHovered(isDark);
    }
    else {
        bgColor = tbBackgroundNormal(isDark);
        frameColor = tbFrameNormal(isDark);
    }

    QPainterPath path;
    QRect rect = opt->rect;
    path.addRoundedRect(rect, frameRound, frameRound);

    painter->fillPath(path, bgColor);

    painter->setPen(QPen(frameColor, 1, Qt::SolidLine));
    painter->drawPath(path);

    QSize pmSize = {10, 2};
    QPointF p;
    p.setX((rect.width() - pmSize.width()) / 2);
    p.setY((rect.height() - pmSize.height()) / 2);
    QPointF p1 {p.x() + rect.x(), p.y() + rect.y()};

    const auto px = StyleHelper::getDotsPixmap(opt);
    if (!px.isNull()) {
        painter->drawPixmap(p1, px);
    }

    painter->restore();
}

} // namespace CUR
