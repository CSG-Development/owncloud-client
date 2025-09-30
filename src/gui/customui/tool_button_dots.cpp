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

    const auto icon = StyleHelper::getDotsIcon(opt);
    icon.paint(painter, rect.adjusted(6, 6, -6, -6), Qt::AlignCenter);

    painter->restore();
}

} // namespace CUR
