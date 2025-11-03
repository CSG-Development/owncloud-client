#include "setupcontroller.h"
#include "gui/customui/stylehelper.h"
#include "gui/folderman.h"
#include "determineauthtypejobfactory.h"
#include "jobs/discoverwebfingerservicejobfactory.h"
#include "jobs/resolveurljobfactory.h"

#include "loginservices/remoteconnector.h"
#include "loginservices/devicelistmanager.h"

#include "theme.h"

#include <QClipboard>
#include <QTimer>
#include <QDir>

using namespace std::chrono_literals;

namespace {

const QString defaultUrlSchemeC = QStringLiteral("https://");
const QStringList supportedUrlSchemesC({ defaultUrlSchemeC, QStringLiteral("http://") });

}

namespace CUR::Wizard {

Q_LOGGING_CATEGORY(lcSetupWizardController, "gui.setupwizard.controller")

SetupController::SetupController(SettingsDialog *parent)
    : QObject(parent)
    , _context(new SetupContext(parent, this))
    , raConnector(new CUR::RemoteConnector(parent))
    , deviceMgr(new CUR::DeviceListManager(parent))
{
    connect(_context->window(), &SetupWidget::rejected, this, [&] {
        qCDebug(lcSetupWizardController) << "wizard window closed";
        Q_EMIT finished(nullptr, SyncMode::Invalid, {});
    });

    connect(_context->window(), &SetupWidget::loginEmailClicked, this, [&](const QString& user) {
        qCDebug(lcSetupWizardController) << "Login email clicked";
        user_ = user;
        window()->displayPage(SetupWidget::SetupPage::PageCredentials);
        window()->showCredPageProgress(true);

        raConnector->start_query(user_);
    });

    connect(raConnector, &RemoteConnector::code_requested, this, [&] {
        qCDebug(lcSetupWizardController) << "Code requested";
        window()->displayPage(SetupWidget::SetupPage::PageCredentials);
        remoteSkipped = false;
        window()->showCodeDialog();
    });

    connect(_context->window(), &SetupWidget::codeEntered, this, [&](const QString& code) {
        qCDebug(lcSetupWizardController) << "Code entered";
        raConnector->query_token(code);
    });

    connect(_context->window(), &SetupWidget::codeSkipped, this, [&] {
        qCDebug(lcSetupWizardController) << "Code skipped";
        remoteSkipped = true;
        deviceMgr->query_local();
    });

    connect(raConnector, &RemoteConnector::fetch_devices_finished, this, [&] {
        qCDebug(lcSetupWizardController) << "Device list query finished";
        remoteDevices = raConnector->deviceList();
        deviceMgr->query_remote(remoteDevices);
    });

    connect(raConnector, &RemoteConnector::error_code, this, [&](int error_code, const QString& str) {
        window()->showErrorMessage(tr("Error code: %1 (%2)").arg(error_code).arg(str));
        window()->showCredPageProgress(false);
    });

    connect(deviceMgr, &DeviceListManager::local_finished, this, [&] {
        qCDebug(lcSetupWizardController) << "Local devices discovery finished";
        localDevices = deviceMgr->mdns_recs();

        fullList = DeviceListManager::combine_lists(localDevices, remoteDevices);

        window()->setDevicesList(fullList);
        window()->showCredPageProgress(false);
    });

    connect(deviceMgr, &DeviceListManager::ra_finished, this, [&] {
        qCDebug(lcSetupWizardController) << "Remote devices discovery finished";
        remoteDevices = deviceMgr->ra_recs();
        deviceMgr->query_local();
    });

    connect(_context->window(), &SetupWidget::refreshDevicesClicked, this, [&] {
        qCDebug(lcSetupWizardController) << "Refresh devices clicked";
        raConnector->start_query(user_);
    });

    connect(_context->window(), &SetupWidget::loginCredentialClicked, this, [&](const QString& url, const QString& user, const QString& password) {
        qCDebug(lcSetupWizardController) << "Login credential clicked" << url;
        url_ = url;
        user_ = user;
        password_ = password;
        startLogin(url, user, password);
    });

    connect(_context->window(), &SetupWidget::credPageBackClicked, this, [&] {
        qCDebug(lcSetupWizardController) << "Login credential back clicked";
        window()->displayPage(SetupWidget::SetupPage::PageEmail);
    });

    connect(this, &SetupController::setupFinishPageDefaults, _context->window(), &SetupWidget::onSetupFinishPageDefaults);

    connect(_context->window(), &SetupWidget::finishPageBackClicked, this, [&] {
        qCDebug(lcSetupWizardController) << "finishPage Back clicked";
        window()->hideErrorMessage();
        window()->displayPage(SetupWidget::SetupPage::PageCredentials);
    });

    connect(_context->window(), &SetupWidget::finishPageDoneClicked, this, [&](CUR::Wizard::SyncMode mode, const QString& targetDir) {
        qCDebug(lcSetupWizardController) << "finishPage Done clicked";
        evaluateFinishPage(mode, targetDir);
    });

    connect(this, &SetupController::credentialsEvaluationFailed, this, [&](const QString& msg) {
        qCWarning(lcSetupWizardController) << "Credentials evaluation failed:" << msg;
        window()->displayPage(SetupWidget::SetupPage::PageCredentials);
        window()->setInvalidCredentialsError();
        window()->showErrorMessage(msg);
    });

    connect(this, &SetupController::invalidServerUrl, this, [&] {
        qCWarning(lcSetupWizardController) << "Invalid server URL";
        window()->displayPage(SetupWidget::SetupPage::PageCredentials);
        window()->setInvalidUrlError();
    });

    connect(this, &SetupController::credentialsEvaluationSuccessful, this, [&] {
        qCWarning(lcSetupWizardController) << "Credentials evaluation successful";
        performLogin();
    });

    connect(this, &SetupController::loginFailed, this, [&](const QString& msg) {
        qCWarning(lcSetupWizardController) << "Login failed:" << msg;
        window()->displayPage(SetupWidget::SetupPage::PageCredentials);
        window()->setInvalidCredentialsError();
        window()->showErrorMessage(msg);
    });

    connect(this, &SetupController::loginSuccessful, this, [&] {
        qCWarning(lcSetupWizardController) << "Login successful";
        window()->displayPage(SetupWidget::SetupPage::PageFinished);
    });

    connect(this, &SetupController::finishSuccessful, this, [&](SyncMode mode) {
        qCWarning(lcSetupWizardController) << "Finish page successful";
        auto account = _context->accountBuilder().build();
        Q_ASSERT(account != nullptr);
        Q_EMIT finished(account, mode, _context->accountBuilder().dynamicRegistrationData());
    });

    connect(this, &SetupController::finishFailed, this, [&](const QString& msg) {
        qCWarning(lcSetupWizardController) << "Finish page failed:" << msg;
        window()->displayPage(SetupWidget::SetupPage::PageFinished);
        window()->showErrorMessage(msg);
    });

    setupFinishPage();
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

void SetupController::startLogin(const QString& url, const QString& user, const QString& password)
{
    window()->displayPage(SetupWidget::SetupPage::PageWait);
    evaluateCredentials(url, user, password);
}

void SetupController::evaluateCredentials(const QString& url, const QString &login, const QString &password)
{
    const QUrl serverUrl = [url]() {
        QString userProvidedUrl = url;

        // fix scheme if necessary
        // using HTTPS as a default is a real ly good idea nowadays, users can still enter http:// explicitly if they wish to
        if (!std::any_of(supportedUrlSchemesC.begin(), supportedUrlSchemesC.end(), [userProvidedUrl](const QString &scheme) {
                return userProvidedUrl.startsWith(scheme);
            })) {
            qInfo(lcSetupWizardController) << "no URL scheme provided, prepending default URL scheme" << defaultUrlSchemeC;
            userProvidedUrl.prepend(defaultUrlSchemeC);
        }

        return QUrl::fromUserInput(userProvidedUrl);
    }();

    // (ab)use the account builder as temporary storage for the URL we are about to probe (after sanitation)
    // in case of errors, the user can just edit the previous value
    _context->accountBuilder().setServerUrl(serverUrl, DetermineAuthTypeJob::AuthType::Unknown);

    // TODO: perform some better validation
    if (!serverUrl.isValid()) {
        Q_EMIT invalidServerUrl();
        return;
    }

    auto *messageBox = new QMessageBox(
        QMessageBox::Warning,
        tr("Insecure connection"),
        tr("The connection to %1 is insecure.\nAre you sure you want to proceed?").arg(serverUrl.toString()),
        QMessageBox::NoButton,
        _context->window());

    messageBox->setAttribute(Qt::WA_DeleteOnClose);

    messageBox->addButton(QMessageBox::Cancel);
    messageBox->addButton(tr("Confirm"), QMessageBox::YesRole);
    StyleHelper::applyPushButtonStyle(messageBox);

    connect(messageBox, &QMessageBox::rejected, this, [this]() {
        Q_EMIT credentialsEvaluationFailed(tr("Insecure server rejected by user"));
    });

    connect(messageBox, &QMessageBox::accepted, this, [this, serverUrl]() {
        // when moving back to this page (or retrying a failed credentials check), we need to make sure existing cookies
        // and certificates are deleted from the access manager
        _context->resetAccessManager();

        // since classic WebFinger is not enabled, we need to check whether modern (oCIS) WebFinger is available
        // therefore, we run the corresponding discovery job
        auto checkWebFingerAuthJob = Jobs::DiscoverWebFingerServiceJobFactory(_context->accessManager()).startJob(serverUrl, this);

        connect(checkWebFingerAuthJob, &CoreJob::finished, this, [job = checkWebFingerAuthJob, serverUrl, this]() {
            // in case any kind of error occurs, we assume the WebFinger service is not available
            if (!job->success()) {
                // first, we must resolve the actual server URL
                auto resolveJob = Jobs::ResolveUrlJobFactory(_context->accessManager()).startJob(serverUrl, this);

                connect(resolveJob, &CoreJob::finished, resolveJob, [this, resolveJob]() {
                    resolveJob->deleteLater();

                    if (!resolveJob->success()) {
                        Q_EMIT credentialsEvaluationFailed(resolveJob->errorMessage());
                        return;
                    }

                    const auto resolvedUrl = resolveJob->result().toUrl();

                    // classic WebFinger workflow: auth type determination is delegated to whatever server the WebFinger service points us to in a dedicated
                    // step we can skip it here therefore
                    if (Theme::instance()->wizardEnableWebfinger()) {
                        _context->accountBuilder().setLegacyWebFingerServerUrl(resolvedUrl);
                        Q_EMIT credentialsEvaluationSuccessful();
                        return;
                    }

                           // next, we need to find out which kind of authentication page we have to present to the user
                    auto authTypeJob = DetermineAuthTypeJobFactory(_context->accessManager()).startJob(resolvedUrl, this);

                    connect(authTypeJob, &CoreJob::finished, authTypeJob, [this, authTypeJob, resolvedUrl]() {
                        authTypeJob->deleteLater();

                        if (authTypeJob->result().isNull()) {
                            Q_EMIT credentialsEvaluationFailed(authTypeJob->errorMessage());
                            return;
                        }

                        _context->accountBuilder().setServerUrl(resolvedUrl, qvariant_cast<DetermineAuthTypeJob::AuthType>(authTypeJob->result()));
                        Q_EMIT credentialsEvaluationSuccessful();
                    });
                });

                connect(
                    resolveJob, &CoreJob::caCertificateAccepted, this,
                    [this](const QSslCertificate &caCertificate) {
                        // future requests made through this access manager should accept the certificate
                        _context->accessManager()->addCustomTrustedCaCertificates({caCertificate});

                               // the account maintains a list, too, which is also saved in the config file
                        _context->accountBuilder().addCustomTrustedCaCertificate(caCertificate);
                    },
                    Qt::DirectConnection);
            } else {
                _context->accountBuilder().setWebFingerAuthenticationServerUrl(job->result().toUrl());
                Q_EMIT credentialsEvaluationSuccessful();
            }

        });
    });

    // instead of defining a lambda that we could call from here as well as the message box, we can put the
    // handler into the accepted() signal handler, and emit that signal here
    if (serverUrl.scheme() == QStringLiteral("https")) {
        Q_EMIT messageBox->accepted();
    } else {
        messageBox->show();
    }

}

void SetupController::performLogin()
{
    _context->accountBuilder().setAuthenticationStrategy(new HttpBasicAuthenticationStrategy(user_, password_));

    if (!_context->accountBuilder().hasValidCredentials()) {
        Q_EMIT loginFailed(tr("Invalid credentials"));
    }

    // not a fan of performing this job here, should be moved into its own (headless) state IMO
    // we can bind it to the current state, which will be cleaned up by changeStateTo(...) as soon as the job finished
    auto fetchUserInfoJob = _context->startFetchUserInfoJob(this);

    connect(fetchUserInfoJob, &CoreJob::finished, this, [this, fetchUserInfoJob] {
        if (fetchUserInfoJob->success()) {
            auto result = fetchUserInfoJob->result().value<FetchUserInfoResult>();
            _context->accountBuilder().setDisplayName(result.displayName());
            _context->accountBuilder().authenticationStrategy()->setDavUser(result.userName());
            Q_EMIT loginSuccessful();
        } else if (fetchUserInfoJob->reply()->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 401) {
            Q_EMIT credentialsEvaluationFailed(tr("Invalid credentials"));
        } else {
            Q_EMIT credentialsEvaluationFailed(tr("Failed to retrieve user information from server"));
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
            Q_EMIT finishFailed(errorMessageTemplate.arg(QStringLiteral("path must be absolute")));
            return;
        }

        QString invalidPathErrorMessage = FolderMan::checkPathValidityRecursive(syncTargetDir);
        if (!invalidPathErrorMessage.isEmpty()) {
            Q_EMIT finishFailed(errorMessageTemplate.arg(invalidPathErrorMessage));
            return;
        }
    }

    Q_EMIT finishSuccessful(mode);
}

SetupController::~SetupController() noexcept
{
}

} // namespace CUR::Wizard
