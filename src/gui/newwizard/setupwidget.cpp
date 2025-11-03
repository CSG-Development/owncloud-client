#include "setupwidget.h"
#include "ui_setupwidget.h"

#include "gui/application.h"
#include "gui/guiutility.h"
#include "gui/curatorgui.h"
#include "gui/settingsdialog.h"
#include "gui/customui/stylehelper.h"
#include "pages/emailpage.h"
#include "pages/credentialspage.h"
#include "pages/waitpage.h"
#include "pages/finishedpage.h"
#include "theme.h"
#include "setupcontext.h"

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

namespace CUR::Wizard {

Q_LOGGING_CATEGORY(lcSetupWizardWidget, "gui.setupwizard.window")

SetupWidget::SetupWidget(SettingsDialog *parent)
    : QWidget(parent)
    , _ui(new ::Ui::SetupWidget)
{
    setWindowFlag(Qt::WindowCloseButtonHint, false);
    setObjectName(QStringLiteral("SetupWidget"));

    _ui->setupUi(this);

    emailPage_ = new EmailPage(this);
    _ui->contentWidget->addWidget(emailPage_);
    connect(emailPage_, &EmailPage::loginClicked, this, &SetupWidget::loginEmailClicked);
    connect(emailPage_, &EmailPage::cancelClicked, this, &SetupWidget::onCancelClicked);

    credPage_ = new CredentialsPage(this);
    _ui->contentWidget->addWidget(credPage_);

    connect(credPage_, &CredentialsPage::cancelClicked, this, &SetupWidget::onCancelClicked);
    connect(credPage_, &CredentialsPage::loginClicked, this, &SetupWidget::loginCredentialClicked);
    connect(credPage_, &CredentialsPage::settingsClicked, this, &SetupWidget::loginSettingsClicked);
    connect(credPage_, &CredentialsPage::resetPasswordClicked, this, &SetupWidget::loginResetPasswordClicked);
    connect(credPage_, &CredentialsPage::refreshDevicesClicked, this, &SetupWidget::refreshDevicesClicked);
    connect(credPage_, &CredentialsPage::backButtonClicked, this, &SetupWidget::credPageBackClicked);
    connect(credPage_, &CredentialsPage::codeEntered, this, &SetupWidget::codeEntered);
    connect(credPage_, &CredentialsPage::codeSkipped, this, &SetupWidget::codeSkipped);

    waitPage_ = new WaitPage(this);
    _ui->contentWidget->addWidget(waitPage_);

    finishPage_ = new FinishedPage(this);
    _ui->contentWidget->addWidget(finishPage_);

    connect(finishPage_, &FinishedPage::doneClicked, this, &SetupWidget::finishPageDoneClicked);
    connect(finishPage_, &FinishedPage::backClicked, this, &SetupWidget::finishPageBackClicked);

    connect(Theme::instance(), &Theme::themeChanged, this, &SetupWidget::onThemeChanged);

    hideErrorMessage();

    onThemeChanged();
    displayPage(SetupPage::PageEmail);
}

void SetupWidget::displayPage(SetupPage page)
{
    switch (page) {
    case SetupPage::PageEmail:
        if (emailPage_)
            _ui->contentWidget->setCurrentWidget(emailPage_);
        break;

    case SetupPage::PageCredentials:
        if (credPage_)
            _ui->contentWidget->setCurrentWidget(credPage_);
        break;

    case SetupPage::PageWait:
        if (waitPage_)
            _ui->contentWidget->setCurrentWidget(waitPage_);
        break;

    case SetupPage::PageFinished:
        if (finishPage_)
            _ui->contentWidget->setCurrentWidget(finishPage_);
        break;
    }

    CuratorGui::raise();
}

void SetupWidget::showErrorMessage(const QString &errorMessage)
{
    if (_ui->contentWidget->currentWidget() == emailPage_)
        emailPage_->showErrorMessage(errorMessage);
    if (_ui->contentWidget->currentWidget() == credPage_)
        credPage_->showErrorMessage(errorMessage);
    else if (_ui->contentWidget->currentWidget() == finishPage_)
        finishPage_->showErrorMessage(errorMessage);
}

void SetupWidget::hideErrorMessage()
{
    if (_ui->contentWidget->currentWidget() == emailPage_)
        emailPage_->showErrorMessage({});
    if (_ui->contentWidget->currentWidget() == credPage_)
        credPage_->showErrorMessage({});
    else if (_ui->contentWidget->currentWidget() == finishPage_)
        finishPage_->showErrorMessage({});
}

void SetupWidget::showCodeDialog()
{
    if (_ui->contentWidget->currentWidget() == credPage_)
        credPage_->showCodeDialog();
}

void SetupWidget::setDevicesList(const QList<Device> &list)
{
    if (_ui->contentWidget->currentWidget() == credPage_)
        credPage_->setDevicesList(list);
}

void SetupWidget::setEmail(const QString &email)
{
    if (_ui->contentWidget->currentWidget() == credPage_)
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
    CuratorGui::raise();
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

void SetupWidget::showCredPageProgress(bool show)
{
    credPage_->showProgressIndicator(show);
}

SetupWidget::~SetupWidget() noexcept
{
    delete _ui;
}

void SetupWidget::onThemeChanged()
{
    bool isDark = CUR::Theme::instance()->isDarkTheme();
    setStyleSheet(StyleHelper::loadFileToString(isDark ? widgetStyle.second : widgetStyle.first));

    if (emailPage_)
        emailPage_->updateTheme();

    if (credPage_)
        credPage_->updateTheme();

    if (waitPage_)
        waitPage_->updateTheme();

    if (finishPage_)
        finishPage_->updateTheme();
}

}
