#include "stylehelper.h"
#include "style_tool_button_menu.h"
#include "style_push_button.h"
#include "libsync/theme.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QStyleOption>
#include <QStyleHints>
#include <QSettings>
#include <QPushButton>

#ifdef Q_OS_DARWIN
static const qreal qstyleBaseDpi = 72;
#else
static const qreal qstyleBaseDpi = 96;
#endif

int qt_defaultDpiX()
{
    if (QCoreApplication::instance()->testAttribute(Qt::AA_Use96Dpi))
        return 96;

    if (const QScreen *screen = QGuiApplication::primaryScreen())
        return qRound(screen->logicalDotsPerInchX());

    //PI has not been initialised, or it is being initialised. Give a default dpi
    return 100;
}

void static loadrc()
{
    Q_INIT_RESOURCE(customui_res);
}

void static unloadrc()
{
    Q_CLEANUP_RESOURCE(customui_res);
}

namespace CUR {

QProxyStyle* StyleHelper::tbMenuStyle_ = nullptr;
QProxyStyle* StyleHelper::pushButtonStyle_ = nullptr;
StyleHelper* StyleHelper::instance_ = nullptr;

StyleHelper::StyleHelper(QObject* parent)
    : QObject(parent)
{
    instance_ = this;
    ::loadrc();

    tbMenuStyle_ = new ProxyStyleToolButtonMenu;
    pushButtonStyle_ = new ProxyStylePushButton;

    setDarkMode(Theme::instance()->isDarkTheme());
}

StyleHelper::~StyleHelper()
{
    ::unloadrc();
    instance_ = nullptr;
}

void StyleHelper::applyPushButtonStyle(QWidget *root)
{
    Q_ASSERT(root);
    const auto& btns = root->findChildren<QPushButton*>();
    for (auto* t: btns) {
        t->setStyle(StyleHelper::pushButtonStyle());
        t->setMinimumHeight(34);
        t->setCursor(Qt::PointingHandCursor);
#ifdef Q_OS_MACOS
        t->setAttribute(Qt::WA_Hover, true);
#endif
    }
}

qreal StyleHelper::dpi(const QStyleOption *option)
{
#ifndef Q_OS_DARWIN
    // Prioritize the application override, except for on macOS where
    // we have historically not supported the AA_Use96Dpi flag.
    if (QCoreApplication::testAttribute(Qt::AA_Use96Dpi))
        return 96;
#endif

    // Expect that QStyleOption::QFontMetrics::QFont has the correct DPI set
    if (option)
        return option->fontMetrics.fontDpi();

    // Fall back to historical Qt behavior: hardocded 72 DPI on mac,
    // primary screen DPI on other platforms.
#ifdef Q_OS_DARWIN
    return qstyleBaseDpi;
#else
    return qt_defaultDpiX();
#endif
}

qreal StyleHelper::dpiScaled(qreal value, qreal dpi)
{
    return value * dpi / qstyleBaseDpi;
}

QIcon StyleHelper::getIcon(const QString &name)
{
    if (name == QStringLiteral("plus-solid"))
        return QIcon(QStringLiteral(":/res/Add.png"));
    else if (name == QStringLiteral("activity"))
        return QIcon(QStringLiteral(":/res/Activity.png"));
    else if (name == QStringLiteral("settings"))
        return QIcon(QStringLiteral(":/res/Settings.png"));
    else if (name == QStringLiteral("account"))
        return QIcon(QStringLiteral(":/res/User.png"));
    else if (name == QStringLiteral("quit"))
        return QIcon(QStringLiteral(":/res/Power.png"));
    return {};
}

QPixmap StyleHelper::getDotsPixmap(const QStyleOptionToolButton *opt)
{
    if ((opt->state & QStyle::State_Enabled) == 0)
        return QPixmap(QStringLiteral(":/res/checkbox/chk_dots_disabled.png"));
    else if (opt->state & QStyle::State_Sunken)
        return QPixmap(QStringLiteral(":/res/checkbox/chk_dots_pressed.png"));
    else if (opt->state & QStyle::State_MouseOver)
        return QPixmap(QStringLiteral(":/res/checkbox/chk_dots_hovered.png"));

    return QPixmap(QStringLiteral(":/res/checkbox/chk_dots.png"));
}

QPixmap StyleHelper::getArrowPixmap(const QStyleOptionToolButton* opt, bool isDark)
{
    bool isDisabled = (opt->state & QStyle::State_Enabled) == 0;
    bool isPressed = (opt->state & QStyle::State_Raised) == 0;
    return getArrowPixmap(opt->arrowType, isPressed, isDisabled, isDark);
}

QPixmap StyleHelper::getArrowPixmap(Qt::ArrowType arrow, bool isPressed, bool isDisabled, bool isDark)
{
    if (arrow == Qt::UpArrow) {
        if (isDisabled)
            return QPixmap(isDark ? QStringLiteral(":/res/arrow/arrow_up_disabled_dark.png") : QStringLiteral(":/res/arrow/arrow_up_disabled.png"));
        if (isPressed)
            return QPixmap(isDark ? QStringLiteral(":/res/arrow/arrow_up_pressed_dark.png") : QStringLiteral(":/res/arrow/arrow_up_pressed.png"));

        return QPixmap(isDark ? QStringLiteral(":/res/arrow/arrow_up_dark.png") : QStringLiteral(":/res/arrow/arrow_up.png"));
    }
    else if (arrow == Qt::DownArrow) {
        if (isDisabled)
            return QPixmap(isDark ? QStringLiteral(":/res/arrow/arrow_down_disabled_dark.png") : QStringLiteral(":/res/arrow/arrow_down_disabled.png"));
        if (isPressed)
            return QPixmap(isDark ? QStringLiteral(":/res/arrow/arrow_down_pressed_dark.png") : QStringLiteral(":/res/arrow/arrow_down_pressed.png"));

        return QPixmap(isDark ? QStringLiteral(":/res/arrow/arrow_down_dark.png") : QStringLiteral(":/res/arrow/arrow_down.png"));
    }

    return {};
}

void StyleHelper::setDarkMode(bool dark)
{
    if (auto s = dynamic_cast<ProxyStyleBase*>(tbMenuStyle_))
        s->setDarkMode(dark);
    if (auto s = dynamic_cast<ProxyStyleBase*>(pushButtonStyle_))
        s->setDarkMode(dark);
}

StyleHelper *StyleHelper::getInstance()
{
    return instance_;
}

} // namespace CUR
