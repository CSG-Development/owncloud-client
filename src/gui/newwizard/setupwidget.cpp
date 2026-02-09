#include "setupwidget.h"
#include "ui_setupwidget.h"

#include "gui/application.h"
#include "gui/guiutility.h"
#include "gui/applicationgui.h"
#include "gui/settingsdialog.h"
#include "gui/customui/stylehelper.h"
#include "pages/emailpage.h"
#include "pages/credentialspage.h"
#include "pages/waitpage.h"
#include "pages/finishedpage.h"
#include "pages/connecterrorpage.h"
#include "pages/pagecontext.h"
#include "theme.h"
#include "setupcontext.h"
#include "configfile.h"

#include <QLabel>
#include <QMessageBox>
#include <QStyleFactory>

using namespace std::chrono_literals;

namespace {
QPair<QString,QString> widgetStyle = {
    QStringLiteral(":/res/login/setupwidget_light.qss"),
    QStringLiteral(":/res/login/setupwidget_dark.qss")
};
}

namespace APP::Wizard {

Q_LOGGING_CATEGORY(lcSetupWizardWidget, "gui.setupwizard.window")

SetupWidget::SetupWidget(SettingsDialog *parent)
    : QWidget(parent)
    , _ui(new ::Ui::SetupWidget)
{
    qRegisterMetaType<PageContext>();

    setWindowFlag(Qt::WindowCloseButtonHint, false);
    setObjectName(QStringLiteral("SetupWidget"));

    _ui->setupUi(this);

    setStyleSheet(StyleHelper::loadFileToString(QStringLiteral(":/res/login/setupwidget.qss")));

    emailPage_ = new EmailPage(this);
    _ui->contentWidget->addWidget(emailPage_);
    connect(emailPage_, &EmailPage::loginClicked, this, &SetupWidget::loginEmailClicked);
    connect(emailPage_, &EmailPage::cancelClicked, this, &SetupWidget::onCancelClicked);

    credPage_ = new CredentialsPage(this);
    _ui->contentWidget->addWidget(credPage_);

    connect(credPage_, &CredentialsPage::actionTriggered, this, &SetupWidget::onCredentialsAction);

    waitPage_ = new WaitPage(this);
    _ui->contentWidget->addWidget(waitPage_);

    finishPage_ = new FinishedPage(this);
    _ui->contentWidget->addWidget(finishPage_);

    connect(finishPage_, &FinishedPage::doneClicked, this, &SetupWidget::finishPageDoneClicked);
    connect(finishPage_, &FinishedPage::backClicked, this, &SetupWidget::finishPageBackClicked);

    connectErrorPage_ = new ConnectErrorPage(this);
    _ui->contentWidget->addWidget(connectErrorPage_);
    connect(connectErrorPage_, &ConnectErrorPage::backClicked, this, &SetupWidget::connectErrorPageBackClicked);
    connect(connectErrorPage_, &ConnectErrorPage::retryClicked, this, &SetupWidget::connectErrorPageRetryClicked);

    connect(Theme::instance(), &Theme::themeChanged, this, &SetupWidget::onThemeChanged);
    onThemeChanged(APP::Theme::instance()->isDarkTheme());

    hideErrorMessage();

    ConfigFile cf;
    emailPage_->setEmail(cf.favoriteEmail());
    displayPage(SetupPage::PageEmail);
}

void SetupWidget::displayPage(SetupPage page, std::optional<GuiContext> ctx)
{
    guiContext = ctx;
    transitionTo(page);
}

void SetupWidget::displayPreviousPage()
{
    transitionTo(previousPage);
}

void SetupWidget::showErrorMessage(const QString &errorMessage)
{
    _ui->contentWidget->currentWidget()->setProperty("errorMessage", errorMessage);
}

void SetupWidget::hideErrorMessage()
{
    showErrorMessage({});
}

void SetupWidget::setEmailIsNotAllowed(bool val)
{
    if (emailPage_)
        emailPage_->setEmailIsNotRegistered(val);
}

void SetupWidget::setDevicesList(const QList<Device> &list)
{
    if (credPage_)
        credPage_->setDevicesList(list);
}

void SetupWidget::setEmail(const QString &email)
{
    if (credPage_)
        credPage_->setEmail(email);
}

void SetupWidget::onCancelClicked()
{
    auto messageBox = new QMessageBox(QMessageBox::Warning,
        tr("Cancel Setup"),
        tr("Do you really want to cancel the account setup?"),
        QMessageBox::Yes | QMessageBox::No, ocApp()->gui()->settingsDialog());
    messageBox->setAttribute(Qt::WA_DeleteOnClose);
    StyleHelper::applyPushButtonStyle(messageBox);
    connect(messageBox, &QMessageBox::accepted, this, [this] {
        Q_EMIT rejected();
    });
    ApplicationGui::raise();
    messageBox->open();
}

void SetupWidget::onSetupFinishPageDefaults(const QString &defaultSyncTargetDir, const QString &userChosenSyncTargetDir, bool vfsIsAvailable, bool enableVfsByDefault, bool vfsModeIsExperimental)
{
    finishPage_->setupPageDefaults(defaultSyncTargetDir, userChosenSyncTargetDir, vfsIsAvailable, enableVfsByDefault, vfsModeIsExperimental);
}

void SetupWidget::setInvalidUrlError()
{
    credPage_->showInvalidUrlError();
}

void SetupWidget::setInvalidCredentialsError()
{
    credPage_->showInvalidCredentialsError();
}

void SetupWidget::setCredErrorMessage(const QString &error, const QString& tooltip)
{
    credPage_->showErrorMessage(error, tooltip);
}

void SetupWidget::showCredPageProgress(bool show)
{
    credPage_->showProgressIndicator(show);
}

void SetupWidget::onCredentialsAction(CredentialsAction action, std::optional<CredentialsContext> ctx)
{
    if (action == CredentialsAction::CancelClicked) {
        onCancelClicked();
        return;
    }
    emit credentialsAction(action, ctx);
}

void SetupWidget::transitionTo(SetupPage newPage)
{
    if (previousPage != newPage)
        previousPage = currentPage_;
    currentPage_ = newPage;

    QMetaObject::invokeMethod(this, [this] {
        processPageChange();
    }, Qt::QueuedConnection);
}

void SetupWidget::processPageChange()
{
    switch (currentPage_) {
    case SetupPage::PageNone:
        break;

    case SetupPage::PageEmail:
        setSafeCurrentWidget(emailPage_);
        if (previousPage == SetupPage::PageCredentials)
            credPage_->showErrorMessage({});
        break;

    case SetupPage::PageCredentials:
        setSafeCurrentWidget(credPage_);
        break;

    case SetupPage::PageWait:
        setSafeCurrentWidget(waitPage_);
        break;

    case SetupPage::PageFinished:
        setSafeCurrentWidget(finishPage_);
        break;

    case SetupPage::PageConnectError:
        setSafeCurrentWidget(connectErrorPage_);
        break;
    }
}

SetupWidget::~SetupWidget() noexcept
{
    delete _ui;
}

void SetupWidget::onThemeChanged(bool isDark)
{
    StyleHelper::setTheme(this, isDark);
    qCDebug(lcSetupWizardWidget) << isDark;
}

void SetupWidget::setSafeCurrentWidget(QWidget *w)
{
    if (w)
        _ui->contentWidget->setCurrentWidget(w);
}

}
