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

Q_LOGGING_CATEGORY(lcStyleHelper, "gui.stylehelper", QtInfoMsg);

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
        t->setAttribute(Qt::WA_MacShowFocusRect, false);
#endif
    }
}

void StyleHelper::setTheme(QWidget* target, bool isDark)
{
    if (!target)
        return;

    target->setProperty("theme", isDark ? QStringLiteral("dark") : QStringLiteral("light"));

    target->style()->unpolish(target);
    target->style()->polish(target);

    for (auto child : target->findChildren<QWidget*>()) {
        child->style()->unpolish(child);
        child->style()->polish(child);
    }
}

QIcon StyleHelper::getIcon(const QString &name, bool isDark)
{
    if (name == QStringLiteral("plus-solid"))
        return QIcon(isDark ? QStringLiteral(":/res/toolbar/dark/Add.svg") : QStringLiteral(":/res/toolbar/light/Add.svg"));
    else if (name == QStringLiteral("activity"))
        return QIcon(isDark ? QStringLiteral(":/res/toolbar/dark/Activity.svg") : QStringLiteral(":/res/toolbar/light/Activity.svg"));
    else if (name == QStringLiteral("settings"))
        return QIcon(isDark ? QStringLiteral(":/res/toolbar/dark/Settings.svg") : QStringLiteral(":/res/toolbar/light/Settings.svg"));
    else if (name == QStringLiteral("account"))
        return QIcon(isDark ? QStringLiteral(":/res/toolbar/dark/User.svg") : QStringLiteral(":/res/toolbar/light/User.svg"));
    else if (name == QStringLiteral("quit"))
        return QIcon(isDark ? QStringLiteral(":/res/toolbar/dark/Power.svg") : QStringLiteral(":/res/toolbar/light/Power.svg"));
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

void StyleHelper::invoke_setDarkTheme_recursive(QWidget *w)
{
    Q_ASSERT(w);

    bool isDark = CUR::Theme::instance()->isDarkTheme();
    const auto& childrenList = w->findChildren<QWidget*>();
    for (auto* widget: childrenList) {
        if (widget->metaObject()->indexOfProperty("darkTheme") != -1) {
            widget->setProperty("darkTheme", isDark);
            qApp->processEvents();
        }
    }
}

StyleHelper *StyleHelper::getInstance()
{
    return instance_;
}

QString StyleHelper::loadFileToString(const QString &fileName)
{
    // Static cache 'filePath' -> 'content'
    static QHash<QString, QString> styleCache;

    if (styleCache.contains(fileName)) {
        return styleCache.value(fileName);
    }

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QString content = QString::fromUtf8(file.readAll());
        styleCache.insert(fileName, content);
        return content;
    }

    qCWarning(lcStyleHelper) << "Unable to load file" << fileName;
    return {};
}

} // namespace CUR
