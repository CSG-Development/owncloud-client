#include "setupcontroller.h"
#include "gui/customui/stylehelper.h"
#include "gui/folderman.h"
#include "gui/application.h"
#include "gui/applicationgui.h"
#include "gui/settingsdialog.h"
#include "determineauthtypejobfactory.h"
#include "jobs/discoverwebfingerservicejobfactory.h"
#include "jobs/resolveurljobfactory.h"

#include "device/devicecontroller.h"
#include "gui/remoteaccess/overlaycontroller.h"

#include "theme.h"
#include "configfile.h"

#include <QClipboard>
#include <QTimer>
#include <QDir>

using namespace std::chrono_literals;

namespace APP::Wizard {

Q_LOGGING_CATEGORY(lcSetupWizardController, "gui.setupwizard.controller")

SetupController::SetupController(SettingsDialog *parent, RunAccountWizardReason reason)
    : QObject(parent)
    , _context(new SetupContext(parent, this))
    , _deviceController(new DeviceController(parent))
    , reason_(reason)
{
    connect(_context->window(), &SetupWidget::rejected, this, [this] {
        qCDebug(lcSetupWizardController) << "wizard window closed";
        Q_EMIT finished(nullptr, SyncMode::Invalid, {});
    });

    id_ = QUuid::createUuid();

    connect(_context->window(), &SetupWidget::credentialsAction, this, &SetupController::onCredentialsAction);
    connect(_context->window(), &SetupWidget::loginEmailClicked, this, &SetupController::onLoginEmailClicked);
    connect(_context->window(), &SetupWidget::finishPageBackClicked, this, &SetupController::onFinishPageBackClicked);
    connect(_context->window(), &SetupWidget::finishPageDoneClicked, this, &SetupController::onFinishPageDoneClicked);
    connect(_context->window(), &SetupWidget::connectErrorPageBackClicked, this, &SetupController::onConnectErrorPageBackClicked);
    connect(_context->window(), &SetupWidget::connectErrorPageRetryClicked, this, &SetupController::onConnectErrorPageRetryClicked);

    connect(this, &SetupController::setupFinishPageDefaults, _context->window(), &SetupWidget::onSetupFinishPageDefaults);
    connect(this, &SetupController::invalidServerUrl, this, &SetupController::onInvalidServerUrl);

    connect(this, &SetupController::handleCredentialsEvaluation, this, &SetupController::onHandleCredentialsEvaluation);
    connect(this, &SetupController::handleLoginResult, this, &SetupController::onHandleLoginResult);
    connect(this, &SetupController::handleFinishResult, this, &SetupController::onHandleFinishResult);

    connect(this, &SetupController::evaluateCredentialsError, this, &SetupController::onEvaluateCredError);

    connect(this, &SetupController::cantFindDevice, this, &SetupController::onCantFindDevice);

    connect(_deviceController, &DeviceController::devices_updated, this, &SetupController::onDevicesUpdated);

    connect(_deviceController, &DeviceController::prepareLoginFinished, this, [this](const Device& device) {
        device_ = device;

        auto id = Device::getBestPathId(device_);

        if (id.has_value()) {
            qCWarning(lcSetupWizardController) << "Device path found, login...";
            evaluateCredentialsNew(id.value());
        }
        else {
            qCWarning(lcSetupWizardController) << "No device path found";
            window()->displayPage(SetupPage::PageConnectError);
        }
    });

    QPointer<OverlayController> oc = ocApp()->gui()->settingsDialog()->overlayController();
    if (!oc) {
        qCDebug(lcSetupWizardController) << "invalid overlay controller";
        // TODO: Fatal?
    }

    connect(oc.get(), &OverlayController::codeEntered, this, [this](const QString &code) {
        _deviceController->enterAccessCode(code, false);
    });

    connect(oc.get(), &OverlayController::resendRequested, this, [this] {
        _deviceController->initAccessCode();
    });

    connect(oc.get(), &OverlayController::processSkipped, this, [this,oc] {
        if (oc)
            oc->hideAll();
        window()->displayPage(SetupPage::PageCredentials);
        window()->showCredPageProgress(false);
    });

    connect(oc.get(), &OverlayController::errorRetry, this, [this,oc](ErrorDialogState state) {
        if (oc) {
            if (state == ErrorDialogState::UnableToConnectInit) {
                window()->displayPage(SetupPage::PageCredentials);
                window()->showCredPageProgress(true);
                _deviceController->initAccessCode();
            }
            else if (state == ErrorDialogState::UnableToConnectToken) {
                oc->retryAccessCode(id_);
            }
        }
    });

    connect(oc.get(), &OverlayController::errorCancel, this, [this,oc](ErrorDialogState state) {
        if (state == ErrorDialogState::UnableToConnectInit) {
            window()->showCredPageProgress(false);
            window()->displayPage(SetupPage::PageCredentials);
        }
        else {
            if (oc)
                oc->requestAccessCode(id_, false);
        }
    });

    connect(oc.get(), &OverlayController::errorOk, this, [this](ErrorDialogState state) {
        if (state == ErrorDialogState::EmailNotRegistered) {
            window()->displayPage(SetupPage::PageEmail);
            window()->setEmailIsNotAllowed(true);
        }
        else {
            window()->showCredPageProgress(false);
            window()->displayPage(SetupPage::PageCredentials);
        }
    });

    connect(_deviceController, &DeviceController::accessCodeRequest, this, [this,oc] {
        qCDebug(lcSetupWizardController) << "accessCodeRequest";
        if (oc)
            oc->requestAccessCode(id_, true);
        else
            qCDebug(lcSetupWizardController) << "overlay controller was destroyed";
    });

    connect(_deviceController, &DeviceController::accessCodeResult, this,
        [this,oc](DeviceController::AccessCodeContext context, int status_code, const QString &errorString, const QString &errorStacktrace) {
            window()->displayPage(SetupPage::PageCredentials);
            window()->showCredPageProgress(false);
            if (status_code == 200) {
                qCDebug(lcSetupWizardController) << "accessCodeResult Accepted";
                ocApp()->gui()->settingsDialog()->overlayController()->hideAll();
                window()->displayPage(SetupPage::PageCredentials);
                window()->showCredPageProgress(true);
                _deviceController->start_new_account();
            } else {
                qCDebug(lcSetupWizardController) << "accessCodeResult Error"
                                                 << (context == DeviceController::AccessCodeContext::Init ? "Init" : "Token")
                                                 << status_code << errorString << errorStacktrace;
                if (oc) {
                    if (status_code == 401) {
                        // from /init - then incorrect email
                        // from /token - then invalid code
                        if (context == DeviceController::AccessCodeContext::Init) {
                            oc->reportError(ErrorDialogState::EmailNotRegistered, id_);
                        }
                        else if (context == DeviceController::AccessCodeContext::Token) {
                            if (errorString.contains(QStringLiteral("expired"))) {
                                oc->expiredAccessCode(id_);
                            }
                            else {
                                oc->invalidAccessCode(id_);
                            }
                        }
                    }
                    else if (status_code == 500) {
                        if (context == DeviceController::AccessCodeContext::Init) {
                            oc->reportError(ErrorDialogState::UnableToConnectInit, id_);
                        }
                        else if (context == DeviceController::AccessCodeContext::Token) {
                            oc->reportError(ErrorDialogState::UnableToConnectToken, id_);
                        }
                    }
                    else if (status_code == 429) {
                        if (context == DeviceController::AccessCodeContext::Init) {
                            oc->reportError(ErrorDialogState::TooManyAttempts, id_);
                        }
                        else if (context == DeviceController::AccessCodeContext::Token) {
                            oc->resendAccessCode(id_);
                        }
                    }
                }
            }
        });

    setupFinishPage();

    if (reason_ == RunAccountWizardReason::RemovedAndNoMoreAccounts) {
        qCDebug(lcSetupWizardController) << "start reason is RemovedAndNoMoreAccounts";
        ConfigFile cf;
        const auto email = cf.favoriteEmail();
        if (!email.isEmpty()) {
            qCDebug(lcSetupWizardController) << "moving to credentials with email" << email;
            onLoginEmailClicked(email);
        }
    }
}

SetupWidget* SetupController::window()
{
    return _context->window();
}

void SetupController::setupFinishPage()
{
    // being pessimistic by default
    bool vfsIsAvailable = false;
    bool enableVfsByDefault = false;
    bool vfsModeIsExperimental = false;

    switch (VfsPluginManager::instance().bestAvailableVfsMode()) {
    case Vfs::WindowsCfApi:
        vfsIsAvailable = true;
        enableVfsByDefault = true;
        vfsModeIsExperimental = false;
        break;
    case Vfs::WithSuffix:
        // we ignore forceVirtualFilesOption if experimental features are disabled
        vfsIsAvailable = Theme::instance()->enableExperimentalFeatures();
        enableVfsByDefault = false;
        vfsModeIsExperimental = true;
        break;
    default:
        break;
    }

    const auto urlToSuggestSyncFolderFor = [this]() {
        const auto selectedInstance = _context->accountBuilder().webFingerSelectedInstance();

        if (!selectedInstance.isEmpty()) {
            return selectedInstance;
        }

        return _context->accountBuilder().serverUrl();
    }();

    QString defaultSyncTargetDir = FolderMan::suggestSyncFolder(urlToSuggestSyncFolderFor, _context->accountBuilder().displayName());
    QString syncTargetDir = _context->accountBuilder().syncTargetDir();

    if (syncTargetDir.isEmpty()) {
        syncTargetDir = defaultSyncTargetDir;
    }

    Q_EMIT setupFinishPageDefaults(defaultSyncTargetDir, syncTargetDir, vfsIsAvailable, enableVfsByDefault, vfsModeIsExperimental);
}

void SetupController::onCredentialsAction(CredentialsAction action, std::optional<CredentialsContext> ctx)
{
    qCDebug(lcSetupWizardController) << "action:" << CredentialsActionToStr(action);

    switch (action) {
    case CredentialsAction::CancelClicked:
        // already processed in SetupWidget
        break;

    case CredentialsAction::LoginClicked:
        {
            window()->displayPage(SetupPage::PageWait);
            if (ctx) {
                password_ = ctx->password;
                if (ctx->device) {
                    device_ = ctx->device.value();
                    _deviceController->prepareLogin(device_);
                }
            }
        }
        break;

    case CredentialsAction::SettingsClicked:
        break;

    case CredentialsAction::ResetPasswordClicked:
        break;

    case CredentialsAction::RefreshDevicesClicked:
        _deviceController->start_new_account();
        break;

    case CredentialsAction::BackButtonClicked:
        window()->displayPage(SetupPage::PageEmail);
        window()->setEmailIsNotAllowed(false);
        break;

    case CredentialsAction::CantFindDeviceClicked:
        emit cantFindDevice(QPrivateSignal());
        break;
    }
}

void SetupController::onLoginEmailClicked(const QString& email)
{
    qCDebug(lcSetupWizardController) << "Login email clicked";
    email_ = email;

    ConfigFile cf;
    cf.setFavoriteEmail(email_);

    window()->setEmail(email_);
    window()->displayPage(SetupPage::PageCredentials);
    window()->showCredPageProgress(true);

    _deviceController->setEmail(email);
    _deviceController->start_new_account();
}

void SetupController::onHandleCredentialsEvaluation(SetupResult result, const QString &msg)
{
    if (result == SetupResult::Success) {
        qCWarning(lcSetupWizardController) << "Credentials evaluation successful";
        performLogin();
        return;
    }
    qCWarning(lcSetupWizardController) << "Credentials evaluation failed:" << msg;
    window()->displayPage(SetupPage::PageCredentials);
    window()->setInvalidCredentialsError();
}

void SetupController::onDevicesUpdated(bool raQueried)
{
    qCDebug(lcSetupWizardController) << "onDevicesUpdated, ra was queried" << raQueried;
    fullList = _deviceController->getDevices();
    window()->setDevicesList(fullList);
    window()->showCredPageProgress(false);

    if (fullList.isEmpty()) {
        qCDebug(lcSetupWizardController) << "Empty device list";

        if (raQueried) {
            qCDebug(lcSetupWizardController) << "RA was queried but returns empty device list";
            window()->displayPage(SetupPage::PageConnectError);
        }
        else {
            qCDebug(lcSetupWizardController) << "Empty device list and RA was not queried, exec Can't find device...";
            emit cantFindDevice(QPrivateSignal());
        }
    }
    else {
        window()->displayPage(SetupPage::PageCredentials);
    }
}

void SetupController::onFinishPageBackClicked()
{
    qCDebug(lcSetupWizardController) << "finishPage Back clicked";
    window()->hideErrorMessage();
    window()->displayPage(SetupPage::PageCredentials);
}

void SetupController::onFinishPageDoneClicked(SyncMode mode, const QString &targetDir)
{
    qCDebug(lcSetupWizardController) << "finishPage Done clicked";
    evaluateFinishPage(mode, targetDir);
}

void SetupController::onConnectErrorPageBackClicked()
{
    qCDebug(lcSetupWizardController) << "connectErrorPage Back clicked";
    window()->hideErrorMessage();
    window()->displayPage(SetupPage::PageCredentials);
    window()->showCredPageProgress(false);
}

void SetupController::onConnectErrorPageRetryClicked()
{
    qCDebug(lcSetupWizardController) << "connectErrorPage Retry clicked";
    window()->displayPage(SetupPage::PageCredentials);
    window()->showCredPageProgress(true);
    _deviceController->start_new_account();
}

void SetupController::onInvalidServerUrl()
{
    qCWarning(lcSetupWizardController) << "Invalid server URL";
    window()->displayPage(SetupPage::PageCredentials);
    window()->setInvalidUrlError();
}

void SetupController::onHandleLoginResult(SetupResult result, const QString &msg)
{
    if (result == SetupResult::Success) {
        qCWarning(lcSetupWizardController) << "Login successful";
        window()->displayPage(SetupPage::PageFinished);
        return;
    }
    qCWarning(lcSetupWizardController) << "Login failed:" << msg;
    window()->displayPage(SetupPage::PageCredentials);
    window()->setInvalidCredentialsError();
    window()->showErrorMessage(msg);
}

void SetupController::onHandleFinishResult(SetupResult result, const QString &msg, SyncMode mode)
{
    if (result == SetupResult::Success) {
        qCWarning(lcSetupWizardController) << "Finish page successful";
        auto account = _context->accountBuilder().build();
        Q_ASSERT(account != nullptr);
        emit finished(account, mode, _context->accountBuilder().dynamicRegistrationData());
        return;
    }
    qCWarning(lcSetupWizardController) << "Finish page failed:" << msg;
    window()->displayPage(SetupPage::PageFinished);
    window()->showErrorMessage(msg);
}

void SetupController::onCantFindDevice()
{
    window()->hideErrorMessage();
    window()->showCredPageProgress(true);
    _deviceController->force_ra_account();
}

void SetupController::evaluateCredentialsNew(const QUuid& id)
{
    qCDebug(lcSetupWizardController) << "evaluateCredentialsNew";
    auto dev_path = device_.findPath(id);
    if (!dev_path) {
        Q_EMIT invalidServerUrl();
        return;
    }

    if (dev_path->about.os_state != QStringLiteral("normal")) {
        QString stateStr = tr("OS state: %1").arg(dev_path->about.os_state.isEmpty() ? tr("<empty>") : dev_path->about.os_state);
        qCDebug(lcSetupWizardController) << stateStr;
        Q_EMIT evaluateCredentialsError(stateStr, QPrivateSignal());
        return;
    }

    if (!dev_path->status.oobe_done) {
        qCDebug(lcSetupWizardController) << "OOBE done" << dev_path->status.oobe_done;
        Q_EMIT evaluateCredentialsError(tr("OOBE is not done"), QPrivateSignal());
        return;
    }

    QUrl serverUrl = QUrl(DevHelpers::makeServerUrl(dev_path->address, dev_path->port, true, dev_path->origin != DeviceOrigin::MDNS));

    _context->accountBuilder().setServerUrl(serverUrl, DetermineAuthTypeJob::AuthType::Unknown);
    qCDebug(lcSetupWizardController) << "ServerUrl" << serverUrl.toString();

    // TODO: perform some better validation
    if (!serverUrl.isValid()) {
        Q_EMIT invalidServerUrl();
        return;
    }

    _context->resetAccessManager();

    // first, we must resolve the actual server URL
    auto resolveJob = Jobs::ResolveUrlJobFactory(_context->accessManager()).startJob(serverUrl, this);

    connect(resolveJob, &CoreJob::finished, resolveJob, [this, resolveJob]() {
        resolveJob->deleteLater();

        if (!resolveJob->success()) {
            Q_EMIT handleCredentialsEvaluation(SetupResult::Fail, resolveJob->errorMessage());
            return;
        }

        const auto resolvedUrl = resolveJob->result().toUrl();

        // next, we need to find out which kind of authentication page we have to present to the user
        auto authTypeJob = DetermineAuthTypeJobFactory(_context->accessManager()).startJob(resolvedUrl, this);

        connect(authTypeJob, &CoreJob::finished, authTypeJob, [this, authTypeJob, resolvedUrl]() {
            authTypeJob->deleteLater();

            if (authTypeJob->result().isNull()) {
                Q_EMIT handleCredentialsEvaluation(SetupResult::Fail, authTypeJob->errorMessage());
                return;
            }

            _context->accountBuilder().setServerUrl(resolvedUrl, qvariant_cast<DetermineAuthTypeJob::AuthType>(authTypeJob->result()));
            _context->accountBuilder().setDevice(device_);
            Q_EMIT handleCredentialsEvaluation(SetupResult::Success);
        });
    });

    connect(resolveJob, &CoreJob::caCertificateAccepted, this, [this](const QSslCertificate &caCertificate) {
        // future requests made through this access manager should accept the certificate
        _context->accessManager()->addCustomTrustedCaCertificates({caCertificate});

        // the account maintains a list, too, which is also saved in the config file
        _context->accountBuilder().addCustomTrustedCaCertificate(caCertificate);
    }, Qt::DirectConnection);
}

void SetupController::onEvaluateCredError(const QString &errStr)
{
    window()->displayPage(SetupPage::PageCredentials);
    window()->setCredErrorMessage(errStr);
}

void SetupController::performLogin()
{
    _context->accountBuilder().setAuthenticationStrategy(new HttpBasicAuthenticationStrategy(email_, password_));

    if (!_context->accountBuilder().hasValidCredentials()) {
        Q_EMIT handleLoginResult(SetupResult::Fail, tr("Invalid credentials"));
    }

    // not a fan of performing this job here, should be moved into its own (headless) state IMO
    // we can bind it to the current state, which will be cleaned up by changeStateTo(...) as soon as the job finished
    auto fetchUserInfoJob = _context->startFetchUserInfoJob(this);

    connect(fetchUserInfoJob, &CoreJob::finished, this, [this, fetchUserInfoJob] {
        if (fetchUserInfoJob->success()) {
            auto result = fetchUserInfoJob->result().value<FetchUserInfoResult>();
            _context->accountBuilder().setDisplayName(result.displayName());
            _context->accountBuilder().authenticationStrategy()->setDavUser(result.userName());
            Q_EMIT handleLoginResult(SetupResult::Success);
        }
        else if (fetchUserInfoJob->reply()->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 401) {
            Q_EMIT handleCredentialsEvaluation(SetupResult::Fail, tr("Invalid credentials"));
        }
        else {
            Q_EMIT handleCredentialsEvaluation(SetupResult::Fail, tr("Failed to retrieve user information from server"));
        }
    });

}

void SetupController::evaluateFinishPage(SyncMode mode, const QString &targetDir)
{
    if (mode != Wizard::SyncMode::ConfigureUsingFolderWizard) {
        QString syncTargetDir = QDir::fromNativeSeparators(targetDir);

        // make sure we remember it now so we can show it to the user again upon failures
        _context->accountBuilder().setSyncTargetDir(syncTargetDir);

        const QString errorMessageTemplate = tr("Invalid local download directory: %1");

        if (!QDir::isAbsolutePath(syncTargetDir)) {
            Q_EMIT handleFinishResult(SetupResult::Fail, errorMessageTemplate.arg(QStringLiteral("path must be absolute")));
            return;
        }

        QString invalidPathErrorMessage = FolderMan::checkPathValidityRecursive(syncTargetDir);
        if (!invalidPathErrorMessage.isEmpty()) {
            Q_EMIT handleFinishResult(SetupResult::Fail, errorMessageTemplate.arg(invalidPathErrorMessage));
            return;
        }
    }

    Q_EMIT handleFinishResult(SetupResult::Success, {}, mode);
}

SetupController::~SetupController() noexcept
{
}

} // namespace APP::Wizard
