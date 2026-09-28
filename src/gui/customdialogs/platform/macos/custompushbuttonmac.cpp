#include "custompushbuttonmac.h"
#include "apppalette.h"

#include <QPainterPath>

namespace {

const double offset = 0.5;
const double highlightOffset = 0.5;
const double highlightPenWidth = 1.2;
const double radius = 8.0;
const auto topHighlightColorAccent = std::pair<QColor,QColor> {
    APP::AppPalette::light(APP::ColorToken::MacAccentTopHighlight),
    APP::AppPalette::dark(APP::ColorToken::MacAccentTopHighlight)
};
const auto topHighlightColorStandard = std::pair<QColor,QColor> {
    APP::AppPalette::light(APP::ColorToken::MacStandardTopHighlight),
    APP::AppPalette::dark(APP::ColorToken::MacStandardTopHighlight)
};
const auto acccentBgColor0 = std::pair<QColor,QColor> {
    APP::AppPalette::light(APP::ColorToken::MacAccentGradientTop),
    //QColor(95, 170, 245)
    APP::AppPalette::dark(APP::ColorToken::MacAccentGradientTop)
};
const auto acccentBgColor1 = std::pair<QColor,QColor> {
    APP::AppPalette::light(APP::ColorToken::MacAccentGradientBottom),
    //QColor(45, 130, 230)
    APP::AppPalette::dark(APP::ColorToken::MacAccentGradientBottom)
};
const auto accentTextColor = std::pair<QColor,QColor> {
    APP::AppPalette::light(APP::ColorToken::OnMacAccent),
    APP::AppPalette::dark(APP::ColorToken::OnMacAccent)
};
const auto standardBgColorNormal = std::pair<QColor,QColor> {
    APP::AppPalette::light(APP::ColorToken::MacStandardBackgroundNormal),
    APP::AppPalette::dark(APP::ColorToken::MacStandardBackgroundNormal)
};
const auto standardBgColorPressed = std::pair<QColor,QColor> {
    APP::AppPalette::light(APP::ColorToken::MacStandardBackgroundPressed),
    APP::AppPalette::dark(APP::ColorToken::MacStandardBackgroundPressed)
};
const auto standardTextColor = std::pair<QColor,QColor> {
    APP::AppPalette::light(APP::ColorToken::MacStandardText),
    APP::AppPalette::dark(APP::ColorToken::MacStandardText)
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
