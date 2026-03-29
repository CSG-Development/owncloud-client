#include "tool_button_dots.h"

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

namespace {
#ifdef Q_OS_WINDOWS
    constexpr int frameRound = 3;
#else
    constexpr int frameRound = 6;
#endif
    constexpr qreal dotRadius = 1.3;
    constexpr qreal dotSpacing = 5.0; // center-to-center distance between dots

    QColor dotsColorForState(const QStyleOptionToolButton *opt, bool isDark)
    {
        if (!(opt->state & QStyle::State_Enabled))
            return isDark ? QColor(0x9E, 0x9E, 0x9E) : QColor(0xBB, 0xBB, 0xBB);
        if (opt->state & QStyle::State_Sunken)
            return isDark ? QColor(0x15, 0x65, 0xC0) : QColor(0x1E, 0x88, 0xE5);
        if (opt->state & QStyle::State_MouseOver)
            return isDark ? QColor(0x42, 0xA5, 0xF5) : QColor(0x14, 0x5C, 0xA4);
        return isDark ? QColor(0x1E, 0x88, 0xE5) : QColor(0x19, 0x76, 0xD2);
    }
}

namespace APP {

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
    QRectF rect = opt->rect;
    path.addRoundedRect(rect, frameRound, frameRound);

    painter->fillPath(path, bgColor);

    painter->setPen(QPen(frameColor, 1, Qt::SolidLine));
    painter->drawPath(path);

    painter->setBrush(dotsColorForState(opt, isDark));
    painter->setPen(Qt::NoPen);

    const QPointF center = QPointF(rect.center());
    painter->drawEllipse(QPointF(center.x() - dotSpacing, center.y()), dotRadius, dotRadius);
    painter->drawEllipse(center, dotRadius, dotRadius);
    painter->drawEllipse(QPointF(center.x() + dotSpacing, center.y()), dotRadius, dotRadius);

    painter->restore();
}

} // namespace APP
