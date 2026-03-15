#include "custompushbuttonmac.h"

#include <QPainterPath>

namespace {

const double offset = 0.5;
const double highlightOffset = 0.5;
const double highlightPenWidth = 1.2;
const double radius = 8.0;
const auto topHighlightColorAccent = std::pair<QColor,QColor> {
    QColor(255, 255, 255, 80),
    QColor(160, 210, 255, 180)
};
const auto topHighlightColorStandard = std::pair<QColor,QColor> {
    QColor(255, 255, 255, 200),
    QColor(255, 255, 255, 25)
};
const auto acccentBgColor0 = std::pair<QColor,QColor> {
    QColor(50, 130, 215),
    //QColor(95, 170, 245)
    QColor(123,189,246)
};
const auto acccentBgColor1 = std::pair<QColor,QColor> {
    QColor(20, 100, 185),
    //QColor(45, 130, 230)
    QColor(94,172,237)
};
const auto accentTextColor = std::pair<QColor,QColor> {
    QColor(255, 255, 255, 223),
    QColor(0, 0, 0, 223)
};
const auto standardBgColorNormal = std::pair<QColor,QColor> {
    QColor(230, 230, 235),
    QColor(60, 60, 62)
};
const auto standardBgColorPressed = std::pair<QColor,QColor> {
    QColor(215, 215, 220),
    QColor(70, 70, 72)
};
const auto standardTextColor = std::pair<QColor,QColor> {
    QColor(30, 30, 30),
    QColor(230, 230, 230)
};

}

CustomPushButtonMac::CustomPushButtonMac(QWidget *parent)
    : QPushButton(parent)
{
    setMinimumHeight(32);
    setMaximumHeight(32);
}

void CustomPushButtonMac::setButtonStyle(CustomPushButtonStyle style)
{
    _buttonStyle = style;
    update();
}

void CustomPushButtonMac::setDarkMode(bool dark)
{
    if (_darkMode != dark) {
        _darkMode = dark;
        update();
    }
}

void CustomPushButtonMac::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const bool isPressed = isDown();
    const QRectF r = this->rect().toRectF().adjusted(offset, offset, -offset, -offset);

    QColor bgColor, topHighlight, textColor;
    QLinearGradient grad(r.topLeft(), r.bottomLeft());

    if (_buttonStyle == CustomPushButtonStyle::Accent) {
        if (isDarkMode()) {
            grad.setColorAt(0, acccentBgColor0.second);
            grad.setColorAt(1, acccentBgColor1.second);
            textColor = accentTextColor.second;
            topHighlight = topHighlightColorAccent.second;
        } else {
            grad.setColorAt(0, acccentBgColor0.first);
            grad.setColorAt(1, acccentBgColor1.first);
            textColor = accentTextColor.first;
            topHighlight = topHighlightColorAccent.first;
        }
    } else if (_buttonStyle == CustomPushButtonStyle::Standard) {
        if (isDarkMode()) {
            bgColor = isPressed ? standardBgColorPressed.second : standardBgColorNormal.second;
            textColor = standardTextColor.second;
            topHighlight = topHighlightColorStandard.second;
        } else {
            bgColor = isPressed ? standardBgColorPressed.first : standardBgColorNormal.first;
            textColor = standardTextColor.first;
            topHighlight = topHighlightColorStandard.first;
        }
    }

    painter.setPen(Qt::NoPen);
    if (_buttonStyle == CustomPushButtonStyle::Accent) {
        painter.setBrush(grad);
    } else if (_buttonStyle == CustomPushButtonStyle::Standard) {
        painter.setBrush(bgColor);
    }
    painter.drawRoundedRect(r, radius, radius);

    if (!isPressed) {
        QPainterPath highlightPath;
        highlightPath.addRoundedRect(r.adjusted(highlightOffset, highlightOffset, -highlightOffset, -highlightOffset), radius, radius);

        painter.setPen(QPen(topHighlight, highlightPenWidth));
        painter.setBrush(Qt::NoBrush);

        painter.setClipRect(r.x(), r.y(), r.width(), radius);
        painter.drawPath(highlightPath);
        painter.setClipping(false);
    }

    painter.setPen(textColor);
    painter.setFont(font());
    painter.drawText(r, Qt::AlignCenter, text());
}
