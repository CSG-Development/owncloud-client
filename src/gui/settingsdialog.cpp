/*
 * Copyright (C) by Daniel Molkentin <danimo@owncloud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "accountmanager.h"
#include "accountsettings.h"
#include "activitywidget.h"
#include "application.h"
#include "applicationgui.h"
#include "configfile.h"
#include "generalsettings.h"
#include "overlaycontroller.h"
#include "theme.h"

#include "customui/dimwidget.h"
#include "customui/menu_toolbutton.h"
#include "customui/stylehelper.h"
#include "customdialogs/custommessagebox.h"
#include "resources/resources.h"

#include <QActionGroup>
#include <QDesktopServices>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QMoveEvent>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QScreen>
#include <QSettings>
#include <QShortcut>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <QWindow>
#include <QWindowStateChangeEvent>

#ifdef Q_OS_MACOS
#include "settingsdialog_mac.h"

void setActivationPolicy(ActivationPolicy policy);
#endif

Q_LOGGING_CATEGORY(lcSettingsDialog, "gui.settingsdialog", QtInfoMsg);

// Define to use custom style
// If not defined - used QToolButton overload with custom paintEvent
#define USE_TOOLBUTTON_STYLE

namespace {
auto minimumSizeHint(const QWidget *w)
{
    const QSize min { 800, 700 }; // When changing this, please check macOS: widgets there have larger insets, so they take up more space.
    const auto screen = w->windowHandle() ? w->windowHandle()->screen() : QApplication::screenAt(QCursor::pos());
    if (screen) {
        const auto availableSize = screen->availableSize();
        if (availableSize.isValid()) {
            // Assume we can use at least 90% of the screen, if the screen is smaller than 800x700 pixels.
            //
            // Note: this means that the wizards have even less space: with the style we use, the
            // wizard tries to fit inside the window. So, if this is a common case that users have
            // such small screens, and the contents of the wizard screen are squashed together (or
            // not shown due to lack of space), we should consider putting that content in a
            // scroll-view.
            return min.boundedTo(availableSize * 0.9);
        }
    }
    return min;
}

const float BUTTONSIZERATIO = 1.618f; // golden ratio

/** display name with two lines that is displayed in the settings
 */
QString shortDisplayNameForSettings(APP::Account *account)
{
    QString user = account->davDisplayName();
    if (user.isEmpty()) {
        user = account->credentials()->user();
    }
    QString host = account->url().host();
    int port = account->url().port();
    if (port > 0 && port != 80 && port != 443) {
        host.append(QLatin1Char(':'));
        host.append(QString::number(port));
    }
    return QStringLiteral("%1\n%2").arg(user, host);
}

QPair<QString,QString> widgetStyle = {
    QStringLiteral(":/res/settingsdialog_light.qss"),
    QStringLiteral(":/res/settingsdialog_dark.qss")
};
}


namespace APP {

class ToolButtonAction : public QWidgetAction
{
    Q_OBJECT
public:
    explicit ToolButtonAction(const QIcon &icon, const QString &text, QObject *parent)
        : QWidgetAction(parent)
    {
        setIcon(icon);
        setText(text);
        setCheckable(true);
    }

    explicit ToolButtonAction(const QString &icon, const QString &text, QObject *parent)
        : QWidgetAction(parent)
    {
        setText(text);
        setIconName(icon);
        setCheckable(true);
    }


    QWidget *createWidget(QWidget *parent) override
    {
        auto toolbar = qobject_cast<QToolBar *>(parent);
        if (!toolbar) {
            // this means we are in the extention menu, no special action here
            return nullptr;
        }

#ifdef USE_TOOLBUTTON_STYLE
        QToolButton* btn = new QToolButton(toolbar);
#else
        MenuToolButton* btn = new MenuToolButton(toolbar);
#endif
        QString objectName = QStringLiteral("settingsdialog_toolbutton_");
        objectName += text();
        btn->setObjectName(objectName);
        btn->setDefaultAction(this);
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        // icon size is fixed, we can't use the toolbars actual size hint as it might not be defined yet
        btn->setMinimumWidth(toolbar->iconSize().height() * BUTTONSIZERATIO);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setAttribute(Qt::WA_Hover, true);
        btn->setStyle(StyleHelper::toolbarMenuStyle());
        _widget = btn;
        return btn;
    }

    QString iconName() const
    {
        return _iconName;
    }

    void setIconName(const QString &iconName)
    {
        if (_iconName != iconName) {
            _iconName = iconName;
            updateIcon();
        }
    }

    void updateIcon()
    {
        // if (!_iconName.isEmpty()) {
        //     setIcon(Resources::getCoreIcon(_iconName));
        // }
        setIcon(StyleHelper::getIcon(_iconName, APP::Theme::instance()->isDarkTheme()));
    }

    QWidget* buttonWidget() const {
        return _widget;
    }

private:
    QString _iconName;
    QWidget* _widget = nullptr;
};

SettingsDialog::SettingsDialog(ApplicationGui *gui, QWidget *parent)
    : QMainWindow(parent)
    , _ui(new Ui::SettingsDialog)
    , _gui(gui)
    , _overlayController(new OverlayController(this))
{
    ConfigFile cfg;
    _ui->setupUi(this);

    connect(Theme::instance(), &Theme::themeChanged, this, [this] {
        StyleHelper::setDarkMode(Theme::instance()->isDarkTheme());
        StyleHelper::invoke_setDarkTheme_recursive(this);
        updateToolbarTheme();
        update();
        _ui->stack->currentWidget()->update();
    });

    static bool prevDarkTheme = Theme::instance()->isDarkTheme();
    StyleHelper::setDarkMode(prevDarkTheme);
    auto themeTimer = new QTimer(this);
    themeTimer->setInterval(700);
    themeTimer->start();
    connect(themeTimer, &QTimer::timeout, this, [] {
        bool nowTheme = Theme::instance()->isDarkTheme();
        if (nowTheme != prevDarkTheme) {
            prevDarkTheme = nowTheme;
            Theme::instance()->emit_theme_change();
        }
    }, Qt::QueuedConnection);

    StyleHelper::applyPushButtonsStyle(this);
    _ui->toolBar->layout()->setContentsMargins(0, 0, 0, 0);
    updateToolbarTheme();

    // People perceive this as a Window, so also make Ctrl+W work
    QAction *closeWindowAction = new QAction(this);
    closeWindowAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+W")));
    connect(closeWindowAction, &QAction::triggered, this, &SettingsDialog::hide);
    addAction(closeWindowAction);

    setObjectName(QStringLiteral("Settings")); // required as group for saveGeometry call
    setWindowTitle(Theme::instance()->appNameGUI());

    _actionGroup = new QActionGroup(this);
    _actionGroup->setExclusive(true);

    auto tbAction = new ToolButtonAction(QStringLiteral("plus-solid"), tr("Add account"), this);
    _addAccountAction = tbAction;
    _addAccountAction->setCheckable(false);
    connect(_addAccountAction, &QAction::triggered, this, [] {
        // don't directly connect here, ocApp might not be defined yet
        ocApp()->gui()->runNewAccountWizard(RunAccountWizardReason::CreateAccoundCommand);
    });
    _ui->toolBar->addAction(_addAccountAction);

    // Note: all the actions have a '\n' because the account name is in two lines and
    // all buttons must have the same size in order to keep a good layout
    tbAction = new ToolButtonAction(QStringLiteral("activity"), tr("Activity"), this);

    _activityAction = tbAction;
    _actionGroup->addAction(_activityAction);
    _ui->toolBar->addAction(_activityAction);

    _activitySettings = new ActivitySettings;
    _ui->stack->addWidget(_activitySettings);
    connect(_activitySettings, &ActivitySettings::guiLog, _gui, [this](const QString &title, const QString &msg) {
        _gui->slotShowOptionalTrayMessage(title, msg);
    });
    _activitySettings->setNotificationRefreshInterval(cfg.notificationRefreshInterval());

    auto generalAction = new ToolButtonAction(QStringLiteral("settings"), tr("Settings"), this);
    _actionGroup->addAction(generalAction);
    _ui->toolBar->addAction(generalAction);
    GeneralSettings *generalSettings = new GeneralSettings;
    _ui->stack->addWidget(generalSettings);
    QObject::connect(generalSettings, &GeneralSettings::showAbout, gui, &ApplicationGui::slotAbout);

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    _ui->toolBar->addWidget(spacer);

    const auto appNameGui = Theme::instance()->appNameGUI();

    const auto& urlButtonsList = Theme::instance()->urlButtons();
    for (const auto &[iconName, name, url] : urlButtonsList) {
        auto urlAction = new ToolButtonAction(Theme::instance()->themeUniversalIcon(QStringLiteral("urlIcons/%1").arg(iconName)), name, this);
        urlAction->setCheckable(false);
        connect(urlAction, &QAction::triggered, this, [url = url] {
            if (!QDesktopServices::openUrl(url)) {
                qWarning(lcSettingsDialog) << "Failed to open" << url;
            }
        });
        _ui->toolBar->addAction(urlAction);
    }

    auto *quitAction = new ToolButtonAction(QStringLiteral("quit"), tr("Quit %1").arg(appNameGui), this);
    quitAction->setCheckable(false);
    connect(quitAction, &QAction::triggered, this, [this, appNameGui] {
        auto box = new CustomMessageBox(this);
        box->setHeaderText(tr("Quit %1").arg(appNameGui))
            .setMessageText(tr("Are you sure you want to quit %1?").arg(appNameGui))
            .setAcceptButtonText(tr("Yes"))
            .setRejectButtonText(tr("Cancel"))
            .setDeleteOnClose(true);
        connect(box, &CustomMessageBox::accepted, qApp, &QCoreApplication::quit, Qt::QueuedConnection);
        box->open();
    });
    _ui->toolBar->addAction(quitAction);

    _actionGroupWidgets.insert(_activityAction, _activitySettings);
    _actionGroupWidgets.insert(generalAction, generalSettings);

    connect(_actionGroup, &QActionGroup::triggered, this, &SettingsDialog::slotSwitchPage);

    connect(AccountManager::instance(), &AccountManager::accountAdded, this, &SettingsDialog::accountAdded);
    connect(AccountManager::instance(), &AccountManager::accountRemoved, this, &SettingsDialog::accountRemoved);
    for (const auto &ai : AccountManager::instance()->accounts()) {
        accountAdded(ai);
    }

    QTimer::singleShot(0, this, &SettingsDialog::showFirstPage);

    QAction *showLogWindow = new QAction(this);
    showLogWindow->setShortcut(QKeySequence(QStringLiteral("F12")));
    connect(showLogWindow, &QAction::triggered, gui, &ApplicationGui::slotToggleLogBrowser);
    addAction(showLogWindow);

    QAction *showLogWindow2 = new QAction(this);
    showLogWindow2->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    connect(showLogWindow2, &QAction::triggered, gui, &ApplicationGui::slotToggleLogBrowser);
    addAction(showLogWindow2);

    customizeStyle();

    connect(Theme::instance(), &Theme::themeChanged, this, &SettingsDialog::onThemeChanged);
    onThemeChanged();

    cfg.restoreGeometry(this);
    _normalGeometry = normalGeometry();
    if (windowState().testFlag(Qt::WindowMaximized)) {
        const auto screen = windowHandle() ? windowHandle()->screen() : QApplication::screenAt(QCursor::pos());
        if (screen) {
            const auto available = screen->availableGeometry();
            const bool looksLikeStaleMaximizedGeometry = qAbs(available.width() - _normalGeometry.width()) < 20
                && qAbs(available.height() - _normalGeometry.height()) < 20;
            if (looksLikeStaleMaximizedGeometry) {
                _normalGeometry = QRect(QPoint(), ::minimumSizeHint(this));
                _normalGeometry.moveCenter(available.center());
            }
        }
    }
    setMinimumSize(::minimumSizeHint(this));
#ifdef Q_OS_MAC
    setActivationPolicy(ActivationPolicy::Accessory);
#endif

    connect(_ui->dialogStack, &QStackedWidget::currentChanged, this, [this] {
        auto *w = _ui->dialogStack->currentWidget();
        if (!w->windowTitle().isEmpty()) {
            if (w->objectName() == QStringLiteral("SetupWidget")) {
                setWindowTitle(w->windowTitle());
            } else {
                setWindowTitle(tr("%1 - %2").arg(Theme::instance()->appNameGUI(), w->windowTitle()));
            }
        } else {
            setWindowTitle(Theme::instance()->appNameGUI());
        }
    });
}

SettingsDialog::~SettingsDialog()
{
    delete _ui;
}

void SettingsDialog::updateToolbarTheme()
{
    const bool isDark = Theme::instance()->isDarkTheme();
#ifdef Q_OS_WINDOWS
    QString styleStr = QStringLiteral(
        "#toolBar {background-color: %1;"
        "border: none;"
        "border-bottom: 1px solid %2;"
        "}"
        );
    _ui->toolBar->setStyleSheet(styleStr
                                    .arg(isDark ? QStringLiteral("#1D1E21") : QStringLiteral("#FFFFFF"))
                                    .arg(isDark ? QStringLiteral("#616161") : QStringLiteral("rgba(203, 205, 211, 1)"))
                                );
#else
    QString styleStr = QStringLiteral(
        "#toolBar {background-color: %1;"
        "border: none;"
        "border-bottom: 1px solid %2;"
        "border-top: 1px solid %3;"
        "}"
        );
    _ui->toolBar->setStyleSheet(styleStr
                                    .arg(isDark ? QStringLiteral("#1D1E21") : QStringLiteral("#FFFFFF"))
                                    .arg(isDark ? QStringLiteral("rgba(97, 97, 97, 1)") : QStringLiteral("rgba(203, 205, 211, 1)"))
                                    .arg(isDark ? QStringLiteral("transparent") : QStringLiteral("#F3F3F3"))
                                );
#endif
}

void SettingsDialog::addModalWidget(QWidget *w)
{
    ApplicationGui::raise();
    if (_ui->dialogStack->indexOf(w) == -1) {
        _ui->dialogStack->addWidget(w);
        _ui->dialogStack->setCurrentWidget(w);
    }
}

void SettingsDialog::requestModality(Account *account)
{
    _ui->toolBar->setEnabled(false);
    if (_modalStack.isEmpty()) {
        if (auto *action = _actionForAccount.value(account)) {
            action->trigger();
        }
    }
    _modalStack.append(account);
    ApplicationGui::raise();
}

void SettingsDialog::ceaseModality(Account *account)
{
    if (_modalStack.contains(account)) {
        _modalStack.removeOne(account);
        if (!_modalStack.isEmpty()) {
            if (auto *action = _actionForAccount.value(_modalStack.first())) {
                action->trigger();
            } else {
                ceaseModality(account);
            }
        }
    }
    _ui->toolBar->setEnabled(_modalStack.isEmpty());
}

AccountSettings *SettingsDialog::accountSettings(Account *account)
{
    return qobject_cast<AccountSettings *>(_actionGroupWidgets.value(_actionForAccount.value(account, {}), {}));
}

QWidget* SettingsDialog::currentPage()
{
    return _ui->stack->currentWidget();
}

void SettingsDialog::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::StyleChange:
    case QEvent::PaletteChange:
    case QEvent::ThemeChange:
        customizeStyle();
        break;
    case QEvent::WindowStateChange: {
        auto *stateEvent = static_cast<QWindowStateChangeEvent *>(e);
        if (stateEvent->oldState().testFlag(Qt::WindowMaximized) && !windowState().testFlag(Qt::WindowMaximized)
            && !windowState().testFlag(Qt::WindowMinimized) && _normalGeometry.isValid()) {
            setGeometry(_normalGeometry);
        }
        break;
    }
    default:
        break;
    }

    QMainWindow::changeEvent(e);
}

void SettingsDialog::maybeCaptureNormalGeometry()
{
    if (!isMaximized() && !isMinimized() && !isFullScreen()) {
        _normalGeometry = geometry();
    }
}

void SettingsDialog::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    QTimer::singleShot(0, this, &SettingsDialog::maybeCaptureNormalGeometry);
}

void SettingsDialog::moveEvent(QMoveEvent *e)
{
    QMainWindow::moveEvent(e);
    QTimer::singleShot(0, this, &SettingsDialog::maybeCaptureNormalGeometry);
}

void SettingsDialog::setVisible(bool visible)
{
    if (!visible)
    {
        ConfigFile cfg;
        cfg.saveGeometry(this);
    }

#ifdef Q_OS_MACOS
    if (visible) {
        setActivationPolicy(ActivationPolicy::Regular);
    } else {
        setActivationPolicy(ActivationPolicy::Accessory);
    }
#endif
    QMainWindow::setVisible(visible);
}

void SettingsDialog::slotSwitchPage(QAction *action)
{
    _ui->stack->setCurrentWidget(_actionGroupWidgets.value(action));
}

void SettingsDialog::showFirstPage()
{
    if (!_accountActions.isEmpty()) {
        _accountActions.first()->trigger();
    } else {
        Q_ASSERT(!_ui->toolBar->actions().isEmpty());
        // the first page is always the add button, so select the second
        _ui->toolBar->actions().at(1)->trigger();
    }
}

void SettingsDialog::showActivityPage()
{
    if (_activityAction) {
        _activityAction->trigger();
    }
}

void SettingsDialog::showIssuesList()
{
    if (!_activityAction)
        return;
    _activityAction->trigger();
    _activitySettings->slotShowIssuesTab();
}

void SettingsDialog::accountAdded(AccountStatePtr accountStatePtr)
{
    if (!accountStatePtr) {
        qWarning(lcSettingsDialog) << "Invalid accountStatePtr";
        return;
    }

    bool brandingSingleAccount = !Theme::instance()->multiAccount();

    if (brandingSingleAccount) {
        _ui->toolBar->removeAction(_addAccountAction);
    }

    ToolButtonAction* accountAction;
    const QPixmap avatar = accountStatePtr->account()->avatar();
    const QString actionText = brandingSingleAccount ? tr("Account") : accountStatePtr->account()->displayName();
    if (avatar.isNull()) {
        accountAction = new ToolButtonAction(QStringLiteral("account"), actionText, this);
    } else {
        const QIcon icon(AvatarJob::makeCircularAvatar(avatar));
        accountAction = new ToolButtonAction(icon, actionText, this);
    }

    _accountActions.append(accountAction);

    if (!brandingSingleAccount) {
        accountAction->setToolTip(accountStatePtr->account()->displayName());
        accountAction->setIconText(shortDisplayNameForSettings(accountStatePtr->account().data()));
    }

    // For single account: the add button is removed, place the account tab as the first item.
    // For multi account: we keep the add button on the left, but place the account(s) right after the add button.
    _ui->toolBar->insertAction(brandingSingleAccount ? _ui->toolBar->actions().at(0) : _ui->toolBar->actions().at(1), accountAction);

    // Fix taborder of inserted account button
    if (const auto tbAction = dynamic_cast<ToolButtonAction*>(_addAccountAction)) {
        auto add = tbAction->buttonWidget();
        auto acc = accountAction->buttonWidget();
        if (add && acc) {
            _ui->toolBar->setTabOrder(add, acc);
        }
    }

    auto accountSettings = new AccountSettings(accountStatePtr, this);
    QString objectName = QStringLiteral("accountSettings_");
    objectName += accountStatePtr->account()->displayName();
    accountSettings->setObjectName(objectName);
    _ui->stack->insertWidget(0 , accountSettings);

    _actionGroup->addAction(accountAction);
    _actionGroupWidgets.insert(accountAction, accountSettings);
    _actionForAccount.insert(accountStatePtr->account().data(), accountAction);
    accountAction->trigger();

    connect(accountSettings, &AccountSettings::folderChanged, _gui, &ApplicationGui::slotFoldersChanged);
    connect(accountSettings, &AccountSettings::showIssuesList, this, &SettingsDialog::showIssuesList);
    connect(accountStatePtr->account().data(), &Account::accountChangedAvatar, this, &SettingsDialog::slotAccountAvatarChanged);
    connect(accountStatePtr->account().data(), &Account::accountPresentationChanged, this, &SettingsDialog::slotAccountPresentationChanged);

    // Refresh immediatly when getting online
    connect(accountStatePtr.data(), &AccountState::isConnectedChanged, _activitySettings,
        [this, accountStatePtr] { _activitySettings->slotRefresh(accountStatePtr); });
    _activitySettings->slotRefresh(accountStatePtr);
}

void SettingsDialog::slotAccountAvatarChanged()
{
    Account *account = static_cast<Account *>(sender());
    if (account && _actionForAccount.contains(account)) {
        QAction *action = _actionForAccount[account];
        if (action) {
            const QPixmap pix = account->avatar();
            if (!pix.isNull()) {
                action->setIcon(AvatarJob::makeCircularAvatar(pix));
            }
        }
    }
}

void SettingsDialog::slotAccountPresentationChanged()
{
    Account *account = static_cast<Account *>(sender());
    if (account && _actionForAccount.contains(account)) {
        QAction *action = _actionForAccount[account];
        if (action) {
            QString displayName = account->displayName();
            action->setText(displayName);
            action->setToolTip(displayName);
            action->setIconText(shortDisplayNameForSettings(account));
        }
    }
}

bool SettingsDialog::isOverlayBusy() const
{
    if (_overlayController)
        return _overlayController->isBusy();
    return false;
}

void SettingsDialog::accountRemoved(AccountStatePtr accountStatePtr)
{
    if (!accountStatePtr) {
        qWarning(lcSettingsDialog) << "Invalid accountStatePtr";
        return;
    }

    while (_modalStack.contains(accountStatePtr->account().data())) {
        ceaseModality(accountStatePtr->account().get());
    }
    if (!Theme::instance()->multiAccount()) {
        _ui->toolBar->insertAction(_activityAction, _addAccountAction);
    }

    for (auto it = _actionGroupWidgets.begin(); it != _actionGroupWidgets.end(); ++it) {
        auto as = qobject_cast<AccountSettings *>(*it);
        if (!as) {
            continue;
        }
        if (as->accountsState() == accountStatePtr) {
            _ui->toolBar->removeAction(it.key());
            _accountActions.removeAll(it.key());

            if (_ui->stack->currentWidget() == it.value()) {
                showFirstPage();
            }

            it.key()->deleteLater();
            it.value()->deleteLater();
            _actionGroupWidgets.erase(it);
            break;
        }
    }

    if (_actionForAccount.contains(accountStatePtr->account().data())) {
        _actionForAccount.remove(accountStatePtr->account().data());
    }
    _activitySettings->slotRemoveAccount(accountStatePtr);
}

void SettingsDialog::onThemeChanged()
{
    bool isDark = APP::Theme::instance()->isDarkTheme();
    setStyleSheet(StyleHelper::loadFileToString(isDark ? widgetStyle.second : widgetStyle.first));
}

void SettingsDialog::customizeStyle()
{
#ifdef USE_TOOLBUTTON_STYLE
    const auto& tbrs = _ui->toolBar->findChildren<QToolButton*>();
    for (auto* t: tbrs) {
        t->setStyle(StyleHelper::toolbarMenuStyle());
        t->setMinimumHeight(92);
        t->setAttribute(Qt::WA_Hover, true);
    }
#endif

    const auto &toolButtonActions = findChildren<ToolButtonAction *>();
    for (auto *a : toolButtonActions) {
        a->updateIcon();
    }
}


} // namespace APP

#include "settingsdialog.moc"
