#include "custom_tooltip.h"
#include "stylehelper.h"
#include "theme.h"

#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QFile>

namespace ToolTipTheme {

struct NonVisualProps {
    int shadowX;
    int shadowY;
    int shadowBlur;
    QColor shadowColor;
    QPoint offset;
};

const NonVisualProps LightProps = { 0, 4, 8, QColor(0, 0, 0, 36), QPoint(16, 16) };
const NonVisualProps DarkProps = { 0, 4, 8, QColor(0, 0, 0, 36), QPoint(16, 16) };

const QPair<QString,QString> widgetStyle = {
    QStringLiteral(":/res/tooltips/tooltip_light.qss"),
    QStringLiteral(":/res/tooltips/tooltip_dark.qss")
};

} // namespace ToolTipTheme


CustomToolTip::CustomToolTip(QWidget */*parent*/)
    : QWidget(nullptr, Qt::ToolTip | Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_ShowWithoutActivating);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(shadowMargins_);

    container = new QFrame(this);
    container->setObjectName("ToolTipContainer");

    label = new QLabel(container);
    label->setObjectName("ToolTipLabel");
    label->setWordWrap(true);

    auto* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->addWidget(label);
    mainLayout->addWidget(container);

    shadowEffect = new QGraphicsDropShadowEffect(this);
    container->setGraphicsEffect(shadowEffect);

    onThemeChanged(APP::Theme::instance()->isDarkTheme());
}

QPoint CustomToolTip::offset() const
{
    return currentOffset;
}

void CustomToolTip::setText(const QString &text)
{
    label->setText(text);
    label->adjustSize();
    adjustSize();
}

QMargins CustomToolTip::shadowMargins() const
{
    return shadowMargins_;
}

void CustomToolTip::doShow()
{
    show();
    label->adjustSize();
    adjustSize();
}

void CustomToolTip::onThemeChanged(bool isDark)
{
    ToolTipTheme::NonVisualProps props = ToolTipTheme::LightProps;
    auto qssPath = ToolTipTheme::widgetStyle.first;
    if (isDark) {
        props = ToolTipTheme::DarkProps;
        qssPath = ToolTipTheme::widgetStyle.second;
    }

    // Unpolish the current style
    this->style()->unpolish(this);
    container->style()->unpolish(container);
    label->style()->unpolish(label);

    setStyleSheet(APP::StyleHelper::loadFileToString(qssPath));

    // Re-polish to apply the new rules
    this->style()->polish(this);
    container->style()->polish(container);
    label->style()->polish(label);

    // Force a layout recalculation
    this->ensurePolished();
    this->update();

    shadowEffect->setXOffset(props.shadowX);
    shadowEffect->setYOffset(props.shadowY);
    shadowEffect->setBlurRadius(props.shadowBlur);
    shadowEffect->setColor(props.shadowColor);
    currentOffset = props.offset;
}
