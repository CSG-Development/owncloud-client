#include "tooltip_manager.h"
#include "custom_tooltip.h"
#include "libsync/theme.h"

#include <QGuiApplication>
#include <QHelpEvent>
#include <QScreen>
#include <QStyle>
#include <QLabel>

ToolTipManager::ToolTipManager(QObject *parent)
    : QObject(parent)
{
    tipWidget = new CustomToolTip();

    delayTimer.setSingleShot(true);
    hideTimer.setSingleShot(true);

    connect(&delayTimer, &QTimer::timeout, this, &ToolTipManager::showTip);
    connect(&hideTimer, &QTimer::timeout, tipWidget, &QWidget::hide);

    connect(CUR::Theme::instance(), &CUR::Theme::themeChanged, this, [this] {
        onThemeChanged(CUR::Theme::instance()->isDarkTheme());
    });
}

ToolTipManager *ToolTipManager::instance()
{
    static ToolTipManager instance;
    return &instance;
}

void ToolTipManager::showText(const QPoint &pos, const QString &text, QWidget *w, const QRect &rect)
{
    if (text.isEmpty()) {
        hideTip();
        return;
    }

    pendingText = text;
    pendingPos = pos;
    // Correct shadow offset
    pendingPos -= QPoint(tipWidget->shadowMargins().left(), tipWidget->shadowMargins().top());
    targetRect = rect;
    activeWidget = w;

    int wakeDelay = w ? w->style()->styleHint(QStyle::SH_ToolTip_WakeUpDelay) : 700;

    delayTimer.start(wakeDelay);
}

bool ToolTipManager::isVisible() const
{
    return tipWidget && tipWidget->isVisible();
}

void ToolTipManager::hideTip()
{
    delayTimer.stop();
    hideTimer.stop();

    if (tipWidget)
        tipWidget->hide();

    activeWidget = nullptr;
    targetRect = QRect();
}

void ToolTipManager::onThemeChanged(bool isDark)
{
    tipWidget->onThemeChanged(isDark);
}

bool ToolTipManager::eventFilter(QObject* obj, QEvent* event)
{
    QWidget* sourceWidget = qobject_cast<QWidget*>(obj);
    if (!sourceWidget)
        return QObject::eventFilter(obj, event);

    switch (event->type()) {
    case QEvent::ToolTip: {
        if (!sourceWidget->toolTip().isEmpty()) {
            pendingText = sourceWidget->toolTip();

            auto* helpEvent = static_cast<QHelpEvent*>(event);
            pendingPos = helpEvent->globalPos();
            pendingPos -= QPoint(tipWidget->shadowMargins().left(), tipWidget->shadowMargins().top());

            activeWidget = sourceWidget;
            targetRect = QRect(); // Clear any manual rect from previous calls

            // Use system delay (SH_ToolTip_WakeUpDelay)
            int delay = sourceWidget->style()->styleHint(QStyle::SH_ToolTip_WakeUpDelay);
            delayTimer.start(delay);
            return true;
        }
        break;
    }

    case QEvent::MouseMove: {
        if (tipWidget->isVisible() && !targetRect.isNull() && activeWidget) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            // Check if mouse has moved outside the specifically defined valid area
            QPoint localPos = activeWidget->mapFromGlobal(mouseEvent->globalPosition().toPoint());
            if (!targetRect.contains(localPos)) {
                hideTip();
            }
        }
        break;
    }

    case QEvent::Leave:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::Wheel:
    case QEvent::WindowDeactivate:
    case QEvent::FocusOut: {
        hideTip();
        break;
    }

    default:
        break;
    }

    return QObject::eventFilter(obj, event);
}

void ToolTipManager::showTip()
{
    if (pendingText.isEmpty())
        return;

    tipWidget->setText(pendingText);

    QPoint pos = pendingPos + tipWidget->offset();

    if (auto screen = QGuiApplication::screenAt(pendingPos)) {

        QRect geom = screen->availableGeometry();

        if (pos.x() + tipWidget->width() > geom.right())
            pos.setX(pendingPos.x() - tipWidget->width());

        if (pos.y() + tipWidget->height() > geom.bottom())
            pos.setY(pendingPos.y() - tipWidget->height());
    }

    tipWidget->move(pos);
    if (!tipWidget->isVisible()) {
        tipWidget->doShow();
    }

    int displayTime = qMax(5000, static_cast<int>(pendingText.length()) * 40);
    hideTimer.start(displayTime);
}
