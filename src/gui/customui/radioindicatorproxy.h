#pragma once

#include "theme.h"
#include <QProxyStyle>
#include <QStyleOption>
#include <QRadioButton>
#include <QApplication>

class RadioIndicatorProxy : public QProxyStyle
{
public:
    using QProxyStyle::QProxyStyle;

    explicit RadioIndicatorProxy(QWidget* parent = nullptr)
        : QProxyStyle()
    {
        setParent(parent);
    }

    int pixelMetric(PixelMetric metric, const QStyleOption* option = nullptr, const QWidget* widget = nullptr) const override
    {
        if (metric == PM_ExclusiveIndicatorWidth || metric == PM_ExclusiveIndicatorHeight) {
            return 20;
        }
        return QProxyStyle::pixelMetric(metric, option, widget);
    }

    void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                       QPainter* painter, const QWidget* widget) const override
    {
        if (element == PE_IndicatorRadioButton) {
            if (!widget)
                return;

            const auto* btn = qobject_cast<const QRadioButton*>(widget);
            if (!btn || !btn->property("class").toString().contains(QStringLiteral("button_radio")))
                return QProxyStyle::drawPrimitive(element, option, painter, widget);

            const bool dark     = APP::Theme::instance()->isDarkTheme();

            const bool checked  = option->state & State_On;
            const bool hovered  = option->state & State_MouseOver;
            const bool pressed  = option->state & State_Sunken;
            const bool disabled = !(option->state & State_Enabled);
            // const bool dark     = widget->property("theme") == QStringLiteral("dark");

            const QIcon& icon = pickIcon(checked, hovered, pressed, disabled, dark);

            icon.paint(painter, option->rect);
            return;
        }
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }

private:
    static QIcon pickIcon(bool checked, bool hovered, bool pressed, bool disabled, bool dark)
    {
        static const auto _ = [] {
            Q_INIT_RESOURCE(customui_res);
            return true;
        }();

        // state: 0=normal, 1=hover, 2=pressed, 3=disabled
        static const QIcon icons[2][2][4] = {
        { // light
            { // unchecked
                QIcon(QStringLiteral(":/res/radiobutton/light/normal/unchecked.svg")),
                QIcon(QStringLiteral(":/res/radiobutton/light/hover/unchecked.svg")),
                QIcon(QStringLiteral(":/res/radiobutton/light/pressed/unchecked.svg")),
                QIcon(QStringLiteral(":/res/radiobutton/light/disabled/unchecked.svg")),
            },
            { // checked
                QIcon(QStringLiteral(":/res/radiobutton/light/normal/checked.svg")),
                QIcon(QStringLiteral(":/res/radiobutton/light/hover/checked.svg")),
                QIcon(QStringLiteral(":/res/radiobutton/light/pressed/checked.svg")),
                QIcon(QStringLiteral(":/res/radiobutton/light/disabled/checked.svg")),
            },
        },
        { // dark
            { /* unchecked dark ... */
                QIcon(QStringLiteral(":/res/radiobutton/dark/normal/unchecked.svg")),
                QIcon(QStringLiteral(":/res/radiobutton/dark/hover/unchecked.svg")),
                QIcon(QStringLiteral(":/res/radiobutton/dark/pressed/unchecked.svg")),
                QIcon(QStringLiteral(":/res/radiobutton/dark/disabled/unchecked.svg")),
            },
            { /* checked dark ... */
                QIcon(QStringLiteral(":/res/radiobutton/dark/normal/checked.svg")),
                QIcon(QStringLiteral(":/res/radiobutton/dark/hover/checked.svg")),
                QIcon(QStringLiteral(":/res/radiobutton/dark/pressed/checked.svg")),
                QIcon(QStringLiteral(":/res/radiobutton/dark/disabled/checked.svg")),
            },
            },
        };
        const int state = disabled ? 3 : pressed ? 2 : hovered ? 1 : 0;
        return icons[dark ? 1 : 0][checked ? 1 : 0][state];
    }
};
