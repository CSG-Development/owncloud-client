#include "tool_button_dots.h"
#include "apppalette.h"

#include <QPainterPath>
#include <QPushButton>
#include <QToolButton>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>

QColor tbFrameFocused(bool isDark) {return APP::AppPalette::color(APP::ColorToken::ButtonFrameFocused, isDark);}

QColor tbFrameNormal(bool isDark) {return APP::AppPalette::color(APP::ColorToken::ButtonFrameNormal, isDark);}
QColor tbFramePressed(bool isDark) {return APP::AppPalette::color(APP::ColorToken::ButtonFramePressed, isDark);}
QColor tbFrameHovered(bool isDark) {return APP::AppPalette::color(APP::ColorToken::ButtonFrameHovered, isDark);}
QColor tbFrameDisabled(bool isDark) {return APP::AppPalette::color(APP::ColorToken::ButtonFrameDisabled, isDark);}

QColor tbBackgroundNormal(bool isDark) {return APP::AppPalette::color(APP::ColorToken::ButtonBackgroundNormal, isDark);}
QColor tbBackgroundPressed(bool isDark) {return APP::AppPalette::color(APP::ColorToken::ButtonBackgroundPressed, isDark);}
QColor tbBackgroundHovered(bool isDark) {return APP::AppPalette::color(APP::ColorToken::ButtonBackgroundHovered, isDark);}
QColor tbBackgroundDisabled(bool isDark) {return APP::AppPalette::color(APP::ColorToken::ButtonBackgroundDisabled, isDark);}

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
            return APP::AppPalette::color(APP::ColorToken::IconButtonDisabled, isDark);
        if (opt->state & QStyle::State_Sunken)
            return APP::AppPalette::color(APP::ColorToken::IconButtonPressed, isDark);
        if (opt->state & QStyle::State_MouseOver)
            return APP::AppPalette::color(APP::ColorToken::IconButtonHover, isDark);
        return APP::AppPalette::color(APP::ColorToken::IconButton, isDark);
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
