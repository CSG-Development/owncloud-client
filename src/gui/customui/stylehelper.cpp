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
#include <QFile>

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
    const QList<QPushButton*> btns = root->findChildren<QPushButton*>();
    for (auto* t: btns) {
        t->setStyle(StyleHelper::pushButtonStyle());
        t->setMinimumHeight(34);
        t->setCursor(Qt::PointingHandCursor);
#ifdef Q_OS_MACOS
        t->setAttribute(Qt::WA_Hover, true);
#endif
    }
}

QIcon StyleHelper::getIcon(const QString &name)
{
    if (name == QStringLiteral("plus-solid"))
        return QIcon(QStringLiteral(":/res/toolbar/Add.svg"));
    else if (name == QStringLiteral("activity"))
        return QIcon(QStringLiteral(":/res/toolbar/Activity.svg"));
    else if (name == QStringLiteral("settings"))
        return QIcon(QStringLiteral(":/res/toolbar/Settings.svg"));
    else if (name == QStringLiteral("account"))
        return QIcon(QStringLiteral(":/res/toolbar/User.svg"));
    else if (name == QStringLiteral("quit"))
        return QIcon(QStringLiteral(":/res/toolbar/Power.svg"));
    return {};
}

QIcon StyleHelper::getDotsIcon(const QStyleOptionToolButton *opt)
{
    if ((opt->state & QStyle::State_Enabled) == 0)
        return QIcon(QStringLiteral(":/res/dots/dots_disabled.svg"));
    else if (opt->state & QStyle::State_Sunken)
        return QIcon(QStringLiteral(":/res/dots/dots_pressed.svg"));
    else if (opt->state & QStyle::State_MouseOver)
        return QIcon(QStringLiteral(":/res/dots/dots_hover.svg"));

    return QIcon(QStringLiteral(":/res/dots/dots_normal.svg"));
}

QIcon StyleHelper::getArrowIcon(Qt::ArrowType arrow, bool isPressed, bool isDisabled, bool isDark)
{
    if (arrow == Qt::UpArrow) {
        if (isDisabled)
            return QIcon(isDark ? QStringLiteral(":/res/arrow/arrow_up_disabled_dark.png") : QStringLiteral(":/res/arrow/arrow_up_disabled.svg"));
        if (isPressed)
            return QIcon(isDark ? QStringLiteral(":/res/arrow/arrow_up_pressed_dark.png") : QStringLiteral(":/res/arrow/arrow_up_pressed.svg"));

        return QIcon(isDark ? QStringLiteral(":/res/arrow/arrow_up_dark.png") : QStringLiteral(":/res/arrow/arrow_up.svg"));
    }
    else if (arrow == Qt::DownArrow) {
        if (isDisabled)
            return QIcon(isDark ? QStringLiteral(":/res/arrow/arrow_down_disabled_dark.png") : QStringLiteral(":/res/arrow/arrow_down_disabled.svg"));
        if (isPressed)
            return QIcon(isDark ? QStringLiteral(":/res/arrow/arrow_down_pressed_dark.png") : QStringLiteral(":/res/arrow/arrow_down_pressed.svg"));

        return QIcon(isDark ? QStringLiteral(":/res/arrow/arrow_down_dark.png") : QStringLiteral(":/res/arrow/arrow_down.svg"));
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

QString StyleHelper::loadFileToString(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray data = file.readAll();
        return QString::fromUtf8(data);
    }

    return {};
}

} // namespace CUR
