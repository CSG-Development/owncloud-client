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

#include "accountstate.h"
#include "account.h"
#include "accountmanager.h"
#include "application.h"
#include "certificates/certificatevalidator.h"
#include "configfile.h"
#include "fetchserversettings.h"

#include "libsync/creds/abstractcredentials.h"
#include "libsync/creds/httpcredentials.h"
#include "libsync/device/networkmonitor.h"
#include "libsync/syncendpointrecovery.h"

#include "gui/remoteaccess/overlaycontroller.h"
#include "gui/quotainfo.h"
#include "gui/settingsdialog.h"
#include "gui/spacemigration.h"
#include "gui/tlserrordialog.h"
#include "gui/customdialogs/custommessagebox.h"

#include "settingsdialog.h"
#include "socketapi/socketapi.h"
#include "theme.h"

#include <QFontMetrics>
#include <QRandomGenerator>
#include <QSettings>
#include <QTimer>
#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
#include <QNetworkInformation>
#endif

using namespace std::chrono;
using namespace std::chrono_literals;

namespace {

inline const QLatin1String userExplicitlySignedOutC()
{
    return QLatin1String("userExplicitlySignedOut");
}
auto supportsSpacesC()
{
    return QLatin1String("supportsSpaces");
}

constexpr auto networkChangeDebounceInterval = 3s;
constexpr auto networkChangeCooldownInterval = 30s;
constexpr auto syncTriggeredRecoveryCooldownInterval = 10s;

QString optionalUuidToString(const std::optional<QUuid> &value)
{
    return value ? value->toString(QUuid::WithoutBraces) : QStringLiteral("<none>");
}

const char *devicePathResolutionOutcomeString(DevicePathResolutionOutcome outcome)
{
    switch (outcome) {
    case DevicePathResolutionOutcome::ResolvedFromCachedPriority:
        return "resolved_cached_priority";
    case DevicePathResolutionOutcome::ResolvedFromFreshRemoteAccess:
        return "resolved_fresh_remote_access";
    case DevicePathResolutionOutcome::ResolvedFromRemoteRelay:
        return "resolved_remote_relay";
    case DevicePathResolutionOutcome::RequiresRemoteAccessPrompt:
        return "requires_remote_access_prompt";
    case DevicePathResolutionOutcome::UnresolvedAfterFullRefresh:
        return "unresolved_after_full_refresh";
    }

    return "unknown";
}

int endpointRecoveryReasonPriority(APP::EndpointRecoveryReason reason)
{
    switch (reason) {
    case APP::EndpointRecoveryReason::Unauthorized:
        return 100;
    case APP::EndpointRecoveryReason::TlsHandshakeFailed:
        return 90;
    case APP::EndpointRecoveryReason::ServerUnavailable:
        return 80;
    case APP::EndpointRecoveryReason::ConnectionRefused:
        [[fallthrough]];
    case APP::EndpointRecoveryReason::HostResolutionFailed:
        [[fallthrough]];
    case APP::EndpointRecoveryReason::Timeout:
        [[fallthrough]];
    case APP::EndpointRecoveryReason::TemporaryNetworkFailure:
        [[fallthrough]];
    case APP::EndpointRecoveryReason::RemoteHostClosed:
        [[fallthrough]];
    case APP::EndpointRecoveryReason::TransportUnreachable:
        return 70;
    case APP::EndpointRecoveryReason::PathSemanticallyInvalid:
        return 10;
    case APP::EndpointRecoveryReason::NonRecoverableSyncError:
        return 0;
    }

    return 0;
}
} // anonymous namespace

namespace APP {

Q_LOGGING_CATEGORY(lcAccountState, "gui.account.state", QtInfoMsg)

// Returns the dialog when one is shown, so callers can attach to signals. If no dialog is shown
// (because there is one already, or the new URL matches the current URL), a nullptr is returned.
UpdateUrlDialog *AccountState::updateUrlDialog(const QUrl &newUrl)
{
    // guard to prevent multiple dialogs
    if (_updateUrlDialog) {
        return nullptr;
    }

    _updateUrlDialog = UpdateUrlDialog::fromAccount(_account, newUrl, ocApp()->gui()->settingsDialog());

    connect(_updateUrlDialog, &UpdateUrlDialog::accepted, this, [=]() {
        //_account->setUrl(newUrl);
        _account->setDevice(Device::MakeStatic(newUrl.toString(), newUrl.toString()));
        Q_EMIT _account->wantsAccountSaved(_account.data());
        Q_EMIT urlUpdated();
    });

    ApplicationGui::raise();
    _updateUrlDialog->open();

    return _updateUrlDialog;
}

AccountState::AccountState(AccountPtr account)
    : QObject()
    , _account(account)
    , _queueGuard(_account->jobQueue())
    , _state(AccountState::Disconnected)
    , _connectionStatus(ConnectionValidator::Undefined)
    , _waitingForNewCredentials(false)
    , _maintenanceToConnectedDelay(1min + minutes(QRandomGenerator::global()->generate() % 4)) // 1-5min delay
{
    qRegisterMetaType<AccountState *>("AccountState*");
    qRegisterMetaType<EndpointRecoveryEvent>("EndpointRecoveryEvent");

    connect(account.data(), &Account::invalidCredentials, this, &AccountState::slotInvalidCredentials);
    connect(account.data(), &Account::credentialsFetched, this, &AccountState::slotCredentialsFetched);
    connect(account.data(), &Account::credentialsAsked, this, &AccountState::slotCredentialsAsked);
    connect(account.data(), &Account::unknownConnectionState, this, [this] {
            checkConnectivity(true);
    });
    connect(account.data(), &Account::requestUrlUpdate, this, &AccountState::updateUrlDialog);
    connect(this, &AccountState::urlUpdated, this, [this] {
        checkConnectivity(false);
    });
    connect(account.data(), &Account::requestUrlUpdate, this, &AccountState::updateUrlDialog, Qt::QueuedConnection);
    connect(this, &AccountState::urlUpdated, this, [this] {
        checkConnectivity(false);
    }, Qt::QueuedConnection);
    connect(this, &AccountState::pathUpdateFinished, this, [this](bool skippedCode, const Device& device) {
        qCDebug(lcAccountState) << "pathUpdateFinished. Skip code" << skippedCode;
        if (skippedCode) {
            const auto generation = _pendingDevicePathUpdate ? _pendingDevicePathUpdate->generation : 0;
            const auto trigger = _pendingDevicePathUpdate ? _pendingDevicePathUpdate->trigger : DeviceUpdateTrigger::Default;
            qCInfo(lcAccountState) << "Finishing pending RA path update without device switch"
                                   << "generation" << generation
                                   << "trigger" << deviceUpdateTriggerString(trigger)
                                   << "reason" << "access_code_skipped_or_cancelled";
            _activeAccessCodeGeneration = 0;
            _remoteAccessPromptRetryTimer.stop();
            _pendingDevicePathUpdate.reset();
            if (_pendingEndpointRecoveryRequest) {
                ++_endpointRecoveryGeneration;
                _pendingEndpointRecoveryRequest.reset();
                setEndpointRecoveryState(EndpointRecoveryState::Failed);
            }
            setUpdateDeviceProgress(false);
            finishStartupDevicePathResolution(true);
            qCDebug(lcAccountState) << "Code skipped, no path update";
            return;
        }

        const auto trigger = _pendingDevicePathUpdate ? _pendingDevicePathUpdate->trigger : DeviceUpdateTrigger::Default;
        _activeAccessCodeGeneration = 0;
        _remoteAccessPromptRetryTimer.stop();
        _pendingDevicePathUpdate.reset();
        resolveAndApplyDevicePath(device, false, trigger);
    });
    _networkChangeDebounceTimer.setSingleShot(true);
    connect(&_networkChangeDebounceTimer, &QTimer::timeout, this, &AccountState::runNetworkTriggeredDeviceUpdate);
    _syncTriggeredRecoveryCooldownTimer.setSingleShot(true);
    connect(&_syncTriggeredRecoveryCooldownTimer, &QTimer::timeout, this, &AccountState::triggerPendingEndpointRecovery);
    _endpointRecoveryRetryTimer.setSingleShot(true);
    connect(&_endpointRecoveryRetryTimer, &QTimer::timeout, this, &AccountState::triggerPendingEndpointRecovery);
    _remoteAccessPromptRetryTimer.setSingleShot(true);
    connect(&_remoteAccessPromptRetryTimer, &QTimer::timeout, this, &AccountState::tryShowRemoteAccessPrompt);

#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
    if (QNetworkInformation::loadDefaultBackend()) {
        connect(QNetworkInformation::instance(), &QNetworkInformation::reachabilityChanged, this, [this](QNetworkInformation::Reachability reachability) {
            switch (reachability) {
            case QNetworkInformation::Reachability::Online:
                [[fallthrough]];
            case QNetworkInformation::Reachability::Site:
                [[fallthrough]];
            case QNetworkInformation::Reachability::Unknown:
                // the connection might not yet be established
                QTimer::singleShot(0, this, [this] {
                    checkConnectivity(false);
                });
                break;
            case QNetworkInformation::Reachability::Disconnected:
                // explicitly set disconnected, this way a successful checkConnectivity call above will trigger a local discover
                if (state() != State::SignedOut) {
                    setState(State::Disconnected);
                }
                [[fallthrough]];
            case QNetworkInformation::Reachability::Local:
                break;
            }
        });
    } else {
        qCWarning(lcAccountState) << "Failed to load QNetworkInformation";
    }
#endif
    // as a fallback and to recover after server issues we also poll
    auto timer = new QTimer(this);
    timer->setInterval(ConnectionValidator::DefaultCallingInterval);
    connect(timer, &QTimer::timeout, this, [this] {
        checkConnectivity(false);
    });
    timer->start();

    connect(account->credentials(), &AbstractCredentials::requestLogout, this, [this] {
        setState(State::SignedOut);
    });

    if (FolderMan::instance()) {
        FolderMan::instance()->socketApi()->registerAccount(account);
    }

    connect(account.data(), &Account::appProviderErrorOccured, this, [](const QString &error) {
        CustomMessageBox *msgBox = new CustomMessageBox(ocApp()->gui()->settingsDialog());
        msgBox->setHeaderText(Theme::instance()->appNameGUI())
            .setMessageText(error)
            .setSingleButton(true)
            .setSingleButtonText(tr("OK"))
            .setDeleteOnClose(true);

        ApplicationGui::raise();
        msgBox->open();
    });

    // Network configuration changed
    connect(NetworkMonitor::instance(), &NetworkMonitor::network_changed, this, [this] {
        scheduleNetworkTriggeredDeviceUpdate();
    }, Qt::QueuedConnection);

    connect(AccountManager::instance(), &AccountManager::applicationHasCreated, this, [this]{
        initializeRA();
    });
}

AccountState::~AccountState()
{
    _networkChangeDebounceTimer.stop();
    _syncTriggeredRecoveryCooldownTimer.stop();
    _endpointRecoveryRetryTimer.stop();
    _remoteAccessPromptRetryTimer.stop();
    ++_devicePathResolutionGeneration;
    _activeAccessCodeGeneration = 0;
    _pendingDevicePathUpdate.reset();
    _pendingEndpointRecoveryRequest.reset();
    _startupDevicePathResolutionInProgress = false;
    _startupConnectivityCheckDeferred = false;
    _startupConnectivityCheckBlockJobs = false;
}

void AccountState::handleEndpointRecoveryRequest(const EndpointRecoveryEvent& event, const QString& folderPath)
{
    if (!_account) {
        qCWarning(lcAccountState) << "Ignoring endpoint recovery request without account";
        return;
    }

    if (event.accountId != _account->uuid()) {
        qCDebug(lcAccountState) << "Ignoring endpoint recovery request for another account" << event.accountId << _account->uuid();
        return;
    }

    qCInfo(lcAccountState).noquote()
        << "endpointRecoveryRequested"
        << QStringLiteral("folder=%1 baseUrl=%2 reason=%3 activePathId=%4 networkError=%5 httpStatus=%6")
               .arg(folderPath, event.baseUrl, endpointRecoveryReasonString(event.reason),
                   event.activePathId ? event.activePathId->toString(QUuid::WithoutBraces) : QStringLiteral("<none>"),
                   QString::number(event.networkError), QString::number(event.httpStatus));

    if (!shouldScheduleEndpointRecovery(event.reason)) {
        qCDebug(lcAccountState) << "Ignoring endpoint recovery request because reason is not recoverable"
                                << "reason" << endpointRecoveryReasonString(event.reason)
                                << "folder" << folderPath
                                << "baseUrl" << event.baseUrl;
        return;
    }

    const auto currentActivePathId = _account->activePath();
    if (event.activePathId && !currentActivePathId.isNull() && event.activePathId.value() != currentActivePathId) {
        qCInfo(lcAccountState) << "Ignoring stale endpoint recovery request for inactive path"
                               << "requestPath" << event.activePathId.value()
                               << "currentPath" << currentActivePathId
                               << "reason" << endpointRecoveryReasonString(event.reason)
                               << "baseUrl" << event.baseUrl;
        return;
    }

    const auto currentDavUrl = _account->davUrl().toString();
    if (!event.baseUrl.isEmpty() && !currentDavUrl.isEmpty() && event.baseUrl != currentDavUrl) {
        qCInfo(lcAccountState) << "Ignoring stale endpoint recovery request for outdated base URL"
                               << "requestBaseUrl" << event.baseUrl
                               << "currentBaseUrl" << currentDavUrl
                               << "reason" << endpointRecoveryReasonString(event.reason);
        return;
    }

    if (!canCoordinateEndpointRecovery(true)) {
        return;
    }

    enqueueEndpointRecoveryRequest(event, folderPath);
}

std::unique_ptr<AccountState> AccountState::loadFromSettings(AccountPtr account, const QSettings &settings)
{
    auto accountState = std::unique_ptr<AccountState>(new AccountState(account));
    const bool userExplicitlySignedOut = settings.value(userExplicitlySignedOutC(), false).toBool();
    if (userExplicitlySignedOut) {
        // see writeToSettings below
        accountState->setState(SignedOut);
    }
    accountState->_supportsSpaces = settings.value(supportsSpacesC(), false).toBool();
    return accountState;
}

std::unique_ptr<AccountState> AccountState::fromNewAccount(AccountPtr account)
{
    return std::unique_ptr<AccountState>(new AccountState(account));
}

void AccountState::writeToSettings(QSettings &settings) const
{
    // The SignedOut state is the only state where the client should *not* ask for credentials, nor
    // try to connect to the server. All other states should transition to Connected by either
    // (re-)trying to make a connection, or by authenticating (AskCredentials). So we save the
    // SignedOut state to indicate that the client should not try to re-connect the next time it
    // is started.
    settings.setValue(userExplicitlySignedOutC(), _state == SignedOut);
    settings.setValue(supportsSpacesC(), _supportsSpaces);
}

AccountPtr AccountState::account() const
{
    return _account;
}

AccountState::ConnectionStatus AccountState::connectionStatus() const
{
    return _connectionStatus;
}

QStringList AccountState::connectionErrors() const
{
    return _connectionErrors;
}

AccountState::State AccountState::state() const
{
    return _state;
}

void AccountState::setState(State state)
{
    const State oldState = _state;
    if (_state != state) {
        qCInfo(lcAccountState) << "AccountState state change: " << _state << "->" << state;
        _state = state;

        if (_state == SignedOut) {
            _connectionStatus = ConnectionValidator::Undefined;
            _connectionErrors.clear();
            ++_devicePathResolutionGeneration;
            _activeAccessCodeGeneration = 0;
            _pendingDevicePathUpdate.reset();
            _pendingEndpointRecoveryRequest.reset();
            ++_endpointRecoveryGeneration;
            _startupDevicePathResolutionAttempted = false;
            _startupDevicePathResolutionInProgress = false;
            _startupConnectivityCheckDeferred = false;
            _startupConnectivityCheckBlockJobs = false;
            _syncTriggeredRecoveryCooldownTimer.stop();
            _endpointRecoveryRetryTimer.stop();
            _remoteAccessPromptRetryTimer.stop();
            _lastSyncTriggeredRecoveryAttempt.invalidate();
            setUpdateDeviceProgress(false);
            if (auto *settingsDialog = ocApp()->gui()->settingsDialog()) {
                if (auto overlay = settingsDialog->overlayController()) {
                    overlay->hideAll();
                }
            }
            setEndpointRecoveryState(EndpointRecoveryState::Idle);
        } else if (oldState == SignedOut && _state == Disconnected) {
            // If we stop being voluntarily signed-out, try to connect and
            // auth right now!
            checkConnectivity();
        } else if (_state == ServiceUnavailable) {
            // Check if we are actually down for maintenance.
            // To do this we must clear the connection validator that just
            // produced the 503. It's finished anyway and will delete itself.
            _connectionValidator->deleteLater();
            _connectionValidator.clear();
            checkConnectivity();
        } else if (_state == NetworkError) {
            // Find another URL
            updateDeviceAccessibility();
        }
    }

    // might not have changed but the underlying _connectionErrors might have
    if (_state == Connected) {
        QTimer::singleShot(0, this, [this, oldState] {
            // ensure the connection validator is done
            _queueGuard.unblock();
            // update capabilites and fetch relevant settings
            _fetchCapabilitiesJob = new FetchServerSettingsJob(account(), this);
            connect(_fetchCapabilitiesJob.get(), &FetchServerSettingsJob::finishedSignal, this, [oldState, this] {
                if (oldState == Connected || _state == Connected) {
                    _fetchCapabilitiesJob.clear();
                    emit isConnectedChanged();
                }
            });
            _fetchCapabilitiesJob->start();
        });
    }
    // don't anounce a state change from connected to connected
    // https://github.com/owncloud/client/commit/2c6c21d7532f0cbba4b768fde47810f6673ed931
    if (oldState != state || state != Connected) {
        emit stateChanged(_state);
    }
}

void AccountState::setEndpointRecoveryState(EndpointRecoveryState state)
{
    const auto stateString = [](EndpointRecoveryState currentState) {
        switch (currentState) {
        case EndpointRecoveryState::Idle:
            return "idle";
        case EndpointRecoveryState::Pending:
            return "pending";
        case EndpointRecoveryState::Resolving:
            return "resolving";
        case EndpointRecoveryState::WaitingForRemoteAccessPrompt:
            return "waiting_ra_prompt";
        case EndpointRecoveryState::Deferred:
            return "deferred";
        case EndpointRecoveryState::Completed:
            return "completed";
        case EndpointRecoveryState::Failed:
            return "failed";
        }

        return "unknown";
    };

    if (_endpointRecoveryState == state) {
        return;
    }

    qCDebug(lcAccountState) << "Endpoint recovery state change:"
                            << stateString(_endpointRecoveryState)
                            << "->" << stateString(state);
    _endpointRecoveryState = state;
}

const char *AccountState::deviceUpdateTriggerString(DeviceUpdateTrigger trigger)
{
    switch (trigger) {
    case DeviceUpdateTrigger::Default:
        return "default";
    case DeviceUpdateTrigger::NetworkChange:
        return "network_change";
    case DeviceUpdateTrigger::SyncTransportFailure:
        return "sync_transport_failure";
    }

    return "unknown";
}

bool AccountState::isSignedOut() const
{
    return _state == SignedOut;
}

void AccountState::signOutByUi()
{
    account()->credentials()->forgetSensitiveData();
    account()->clearCookieJar();
    setState(SignedOut);
    // persist that we are signed out
    Q_EMIT account()->wantsAccountSaved(account().data());
}

void AccountState::freshConnectionAttempt()
{
    if (isConnected())
        setState(Disconnected);
    checkConnectivity();
}

void AccountState::signIn()
{
    if (_state == SignedOut) {
        _waitingForNewCredentials = false;
        setState(Disconnected);
        // persist that we are no longer signed out
        Q_EMIT account()->wantsAccountSaved(account().data());
    }
}

bool AccountState::isConnected() const
{
    return _state == Connected;
}

void AccountState::tagLastSuccessfullETagRequest(const QDateTime &tp)
{
    _timeOfLastETagCheck = tp;
}

void AccountState::checkConnectivity(bool blockJobs)
{
    if (isSignedOut() || _waitingForNewCredentials) {
        return;
    }

    if (_startupDevicePathResolutionInProgress) {
        _startupConnectivityCheckDeferred = true;
        _startupConnectivityCheckBlockJobs = _startupConnectivityCheckBlockJobs || blockJobs;
        qCDebug(lcAccountState) << "Deferring connectivity check while startup device path resolution is in progress"
                                << "blockJobs" << blockJobs;
        return;
    }

    if (shouldResolveStartupDevicePath()) {
        startStartupDevicePathResolution(blockJobs);
        return;
    }

    qCInfo(lcAccountState) << "checkConnectivity blocking:" << blockJobs << account()->displayName();
    if (_state != Connected) {
        setState(Connecting);
    }
    if (_tlsDialog) {
        qCDebug(lcAccountState) << "Skip checkConnectivity, waiting for tls dialog";
        return;
    }

    if (_connectionValidator && blockJobs && !_queueGuard.queue()->isBlocked()) {
        // abort already running non blocking validator
        resetConnectionValidator();
    }
    if (_connectionValidator) {
        qCWarning(lcAccountState) << "ConnectionValidator already running, ignoring" << account()->displayName()
                                  << "Queue is blocked:" << _queueGuard.queue()->isBlocked();
        return;
    }

    // If we never fetched credentials, do that now - otherwise connection attempts
    // make little sense.
    if (!account()->credentials()->wasFetched()) {
        _waitingForNewCredentials = true;
        account()->credentials()->fetchFromKeychain();
    }
    if (account()->hasCapabilities()) {
        // IF the account is connected the connection check can be skipped
        // if the last successful etag check job is not so long ago.
        // TODO: https://github.com/owncloud/client/issues/10935
        const auto pta = account()->capabilities().remotePollInterval();
        const auto polltime = duration_cast<seconds>(ConfigFile().remotePollInterval(pta));
        const auto elapsed = _timeOfLastETagCheck.secsTo(QDateTime::currentDateTimeUtc());
        if (!blockJobs && isConnected() && _timeOfLastETagCheck.isValid()
            && elapsed <= polltime.count()) {
            qCDebug(lcAccountState) << account()->displayName() << "The last ETag check succeeded within the last " << polltime.count() << "s (" << elapsed << "s). No connection check needed!";
            return;
        }
    }

    if (blockJobs) {
        _queueGuard.block();
    }
    _connectionValidator = new ConnectionValidator(_account, this);
    connect(_connectionValidator, &ConnectionValidator::connectionResult, this, &AccountState::slotConnectionValidatorResult);
    connect(_connectionValidator, &ConnectionValidator::sslErrors, this, [blockJobs, this](const QList<QSslError> &errors) {
        if (_tlsDialog)
            return;

        // ignore errors for already accepted certificates
        auto filteredErrors = _account->accessManager()->filterSslErrors(errors);

        if (filteredErrors.isEmpty())
            return;

        QSet<QSslCertificate> certs;
        certs.reserve(filteredErrors.size());
        for (const auto &error : std::as_const(filteredErrors)) {
            certs << error.certificate();
        }

        auto acceptCertsAndRestart = [this, certs, blockJobs]() {
            _account->addApprovedCerts(certs);
            resetConnectionValidator();
            checkConnectivity(blockJobs);
        };

        CertificateValidator validator(this);
        bool pinned_valid = validator.validatePinnedCertificate(certs.values());

        if (pinned_valid) {
            qCDebug(lcAccountState) << "Pinned cert match";
            acceptCertsAndRestart();
        }
        else {
            qCDebug(lcAccountState) << "Certificate is not found in pinned list, show TLS dialog";

            _tlsDialog = new TlsErrorDialog(filteredErrors, _account->url().host(), ocApp()->gui()->settingsDialog());
            _tlsDialog->setAttribute(Qt::WA_DeleteOnClose);

            connect(_tlsDialog, &TlsErrorDialog::accepted, _tlsDialog, [this, acceptCertsAndRestart]() {
                acceptCertsAndRestart();
                _tlsDialog.clear();
                scheduleEndpointRecoveryRetry(0);
            });
            connect(_tlsDialog, &TlsErrorDialog::rejected, this, [certs, this]() {
                setState(SignedOut);
            });

            ApplicationGui::raise();
            _tlsDialog->open();

        }
    });

    ConnectionValidator::ValidationMode mode = ConnectionValidator::ValidationMode::ValidateAuthAndUpdate;
    if (isConnected()) {
        // Use a small authed propfind as a minimal ping when we're
        // already connected.
        if (blockJobs) {
            _connectionValidator->setClearCookies(true);
            mode = ConnectionValidator::ValidationMode::ValidateAuth;
        } else {
            mode = ConnectionValidator::ValidationMode::ValidateAuthAndUpdate;
        }
    } else {
        // Check the server and then the auth.
        if (_waitingForNewCredentials) {
            mode = ConnectionValidator::ValidationMode::ValidateServer;
        } else {
            _connectionValidator->setClearCookies(true);
            mode = ConnectionValidator::ValidationMode::ValidateAuthAndUpdate;
        }
    }
    _connectionValidator->checkServer(mode);
}

bool AccountState::shouldResolveStartupDevicePath() const
{
    if (_startupDevicePathResolutionAttempted || _startupDevicePathResolutionInProgress) {
        return false;
    }

    if (!canStartDeviceAccessibilityUpdate(false)) {
        return false;
    }

    const auto device = accountDevice();
    if (!device || device->isStatic) {
        return false;
    }

    return device->paths.size() > 1;
}

void AccountState::startStartupDevicePathResolution(bool blockJobs)
{
    const auto device = accountDevice();
    if (!device) {
        return;
    }

    _startupDevicePathResolutionAttempted = true;
    _startupDevicePathResolutionInProgress = true;
    _startupConnectivityCheckDeferred = true;
    _startupConnectivityCheckBlockJobs = _startupConnectivityCheckBlockJobs || blockJobs;

    qCInfo(lcAccountState) << "Starting startup device path resolution before connectivity check"
                           << "blockJobs" << blockJobs
                           << "currentActivePath" << _account->activePath()
                           << "currentDavUrl" << _account->davUrl();
    if (_state != Connected) {
        setState(Connecting);
    }
    setUpdateDeviceProgress(true);
    resolveAndApplyDevicePath(*device, true, DeviceUpdateTrigger::Default);
}

void AccountState::finishStartupDevicePathResolution(bool continueConnectivity)
{
    if (!_startupDevicePathResolutionInProgress) {
        return;
    }

    const auto shouldContinueConnectivity = continueConnectivity && _startupConnectivityCheckDeferred;
    const auto blockJobs = _startupConnectivityCheckBlockJobs;
    _startupDevicePathResolutionInProgress = false;
    _startupConnectivityCheckDeferred = false;
    _startupConnectivityCheckBlockJobs = false;

    if (shouldContinueConnectivity && !isSignedOut()) {
        qCInfo(lcAccountState) << "Continuing deferred connectivity check after startup device path resolution"
                               << "blockJobs" << blockJobs;
        QTimer::singleShot(0, this, [this, blockJobs] {
            checkConnectivity(blockJobs);
        });
    }
}

void AccountState::resetConnectionValidator()
{
    if (_connectionValidator) {
        _connectionValidator->deleteLater();
        _connectionValidator.clear();
    }
}

void AccountState::updateDeviceAccessibility()
{
    if (!canStartDeviceAccessibilityUpdate(true)) {
        return;
    }

    if (_pendingEndpointRecoveryRequest) {
        qCDebug(lcAccountState) << "Skipping default device availability update because endpoint recovery is pending";
        return;
    }

    qCDebug(lcAccountState) << "Update device availability started";
    setUpdateDeviceProgress(true);
    resolveAndApplyDevicePath(*_account->devicePtr(), true, DeviceUpdateTrigger::Default);
}

bool AccountState::canStartDeviceAccessibilityUpdate(bool logReason) const
{
    if (!_account) {
        if (logReason)
            qCWarning(lcAccountState) << "No account for device availability check";
        return false;
    }

    if (_updateDeviceInProgress) {
        if (logReason)
            qCDebug(lcAccountState) << "Device availability check already in progress";
        return false;
    }

    const auto device = accountDevice();
    if (device && device->isStatic) {
        if (logReason)
            qCDebug(lcAccountState) << "Static device, availability check disabled";
        return false;
    }

    if (ocApp()->gui()->isAccountWizardActive()) {
        if (logReason)
            qCDebug(lcAccountState) << "Skip device availability check, account wizard in progress";
        return false;
    }

    if (isSignedOut() || _waitingForNewCredentials) {
        if (logReason)
            qCDebug(lcAccountState) << "Skip device availability check, signed out";
        return false;
    }

    if (_tlsDialog) {
        if (logReason)
            qCDebug(lcAccountState) << "Skip device availability check, waiting for tls dialog";
        return false;
    }

    if (!_deviceController) {
        if (logReason)
            qCWarning(lcAccountState) << "No device controller created";
        return false;
    }

    return true;
}

bool AccountState::canCoordinateEndpointRecovery(bool logReason) const
{
    if (!_account) {
        if (logReason)
            qCWarning(lcAccountState) << "Skip endpoint recovery coordination, no account";
        return false;
    }

    const auto device = accountDevice();
    if (device && device->isStatic) {
        if (logReason)
            qCDebug(lcAccountState) << "Skip endpoint recovery coordination, static device";
        return false;
    }

    if (isSignedOut()) {
        if (logReason)
            qCDebug(lcAccountState) << "Skip endpoint recovery coordination, account is signed out";
        return false;
    }

    if (!_deviceController) {
        if (logReason)
            qCWarning(lcAccountState) << "Skip endpoint recovery coordination, no device controller";
        return false;
    }

    return true;
}

void AccountState::scheduleEndpointRecoveryRetry(int delayMs)
{
    if (!_pendingEndpointRecoveryRequest || _endpointRecoveryState != EndpointRecoveryState::Deferred) {
        return;
    }

    if (delayMs <= 0) {
        _endpointRecoveryRetryTimer.stop();
        QTimer::singleShot(0, this, &AccountState::triggerPendingEndpointRecovery);
        return;
    }

    _endpointRecoveryRetryTimer.start(delayMs);
}

void AccountState::enqueueEndpointRecoveryRequest(const EndpointRecoveryEvent& event, const QString& folderPath)
{
    const auto shouldReplace = !_pendingEndpointRecoveryRequest || shouldReplacePendingEndpointRecoveryRequest(event);

    if (!shouldReplace) {
        qCInfo(lcAccountState) << "Keeping existing endpoint recovery request"
                               << "pendingGeneration" << _pendingEndpointRecoveryRequest->generation
                               << "pendingReason" << endpointRecoveryReasonString(_pendingEndpointRecoveryRequest->event.reason)
                               << "newReason" << endpointRecoveryReasonString(event.reason)
                               << "folder" << folderPath;
        return;
    }

    if (_pendingEndpointRecoveryRequest) {
        qCInfo(lcAccountState) << "Replacing pending endpoint recovery request"
                               << "oldGeneration" << _pendingEndpointRecoveryRequest->generation
                               << "oldReason" << endpointRecoveryReasonString(_pendingEndpointRecoveryRequest->event.reason)
                               << "newReason" << endpointRecoveryReasonString(event.reason)
                               << "folder" << folderPath;
    }

    const auto nextGeneration = ++_endpointRecoveryGeneration;
    _pendingEndpointRecoveryRequest = PendingEndpointRecoveryRequest {event, folderPath, nextGeneration};
    qCInfo(lcAccountState) << "Queued endpoint recovery request"
                           << "generation" << nextGeneration
                           << "reason" << endpointRecoveryReasonString(event.reason)
                           << "folder" << folderPath
                           << "activePathId" << optionalUuidToString(event.activePathId)
                           << "baseUrl" << event.baseUrl;

    if (_updateDeviceInProgress || _endpointRecoveryState == EndpointRecoveryState::Resolving
        || _endpointRecoveryState == EndpointRecoveryState::WaitingForRemoteAccessPrompt) {
        qCDebug(lcAccountState) << "Deferring queued endpoint recovery request because another update flow is active"
                                << "generation" << nextGeneration
                                << "reason" << endpointRecoveryReasonString(event.reason);
        setEndpointRecoveryState(EndpointRecoveryState::Deferred);
        scheduleEndpointRecoveryRetry(1000);
        return;
    }

    setEndpointRecoveryState(EndpointRecoveryState::Pending);
    QTimer::singleShot(0, this, &AccountState::triggerPendingEndpointRecovery);
}

bool AccountState::shouldReplacePendingEndpointRecoveryRequest(const EndpointRecoveryEvent& event) const
{
    if (!_pendingEndpointRecoveryRequest) {
        return true;
    }

    const auto pendingPriority = endpointRecoveryReasonPriority(_pendingEndpointRecoveryRequest->event.reason);
    const auto newPriority = endpointRecoveryReasonPriority(event.reason);
    if (newPriority != pendingPriority) {
        return newPriority > pendingPriority;
    }

    if (_pendingEndpointRecoveryRequest->event.timestampUtc.isValid() && event.timestampUtc.isValid()) {
        return event.timestampUtc >= _pendingEndpointRecoveryRequest->event.timestampUtc;
    }

    return event.activePathId != _pendingEndpointRecoveryRequest->event.activePathId
        || event.baseUrl != _pendingEndpointRecoveryRequest->event.baseUrl
        || event.networkError != _pendingEndpointRecoveryRequest->event.networkError
        || event.httpStatus != _pendingEndpointRecoveryRequest->event.httpStatus;
}

void AccountState::triggerPendingEndpointRecovery()
{
    if (!_pendingEndpointRecoveryRequest) {
        return;
    }

    if (!_account) {
        qCWarning(lcAccountState) << "Dropping pending endpoint recovery request without account"
                                  << "generation" << _pendingEndpointRecoveryRequest->generation;
        _pendingEndpointRecoveryRequest.reset();
        setEndpointRecoveryState(EndpointRecoveryState::Failed);
        return;
    }

    const auto pendingRequest = *_pendingEndpointRecoveryRequest;
    const auto currentActivePathId = _account->activePath();
    if (pendingRequest.event.activePathId && !currentActivePathId.isNull()
        && pendingRequest.event.activePathId.value() != currentActivePathId) {
        qCInfo(lcAccountState) << "Dropping stale pending endpoint recovery request for inactive path"
                               << "generation" << pendingRequest.generation
                               << "requestPath" << pendingRequest.event.activePathId.value()
                               << "currentPath" << currentActivePathId;
        ++_endpointRecoveryGeneration;
        _pendingEndpointRecoveryRequest.reset();
        setEndpointRecoveryState(EndpointRecoveryState::Completed);
        return;
    }

    const auto currentDavUrl = _account->davUrl().toString();
    if (!pendingRequest.event.baseUrl.isEmpty() && !currentDavUrl.isEmpty()
        && pendingRequest.event.baseUrl != currentDavUrl) {
        qCInfo(lcAccountState) << "Dropping stale pending endpoint recovery request for outdated base URL"
                               << "generation" << pendingRequest.generation
                               << "requestBaseUrl" << pendingRequest.event.baseUrl
                               << "currentBaseUrl" << currentDavUrl;
        ++_endpointRecoveryGeneration;
        _pendingEndpointRecoveryRequest.reset();
        setEndpointRecoveryState(EndpointRecoveryState::Completed);
        return;
    }

    const auto device = accountDevice();
    if (device && device->isStatic) {
        qCDebug(lcAccountState) << "Dropping pending endpoint recovery request for static device"
                                << "generation" << _pendingEndpointRecoveryRequest->generation;
        _pendingEndpointRecoveryRequest.reset();
        setEndpointRecoveryState(EndpointRecoveryState::Failed);
        return;
    }

    if (isSignedOut() || !_deviceController) {
        qCDebug(lcAccountState) << "Dropping pending endpoint recovery request because account is unavailable"
                                << "generation" << _pendingEndpointRecoveryRequest->generation
                                << "signedOut" << isSignedOut()
                                << "hasController" << (_deviceController != nullptr);
        _pendingEndpointRecoveryRequest.reset();
        setEndpointRecoveryState(EndpointRecoveryState::Failed);
        return;
    }

    if (_waitingForNewCredentials || _tlsDialog || ocApp()->gui()->isAccountWizardActive()) {
        qCDebug(lcAccountState) << "Deferring pending endpoint recovery request due to temporary UI/auth state"
                                << "generation" << _pendingEndpointRecoveryRequest->generation
                                << "waitingForCredentials" << _waitingForNewCredentials
                                << "tlsDialog" << bool(_tlsDialog)
                                << "wizardActive" << ocApp()->gui()->isAccountWizardActive();
        setEndpointRecoveryState(EndpointRecoveryState::Deferred);
        scheduleEndpointRecoveryRetry(1000);
        return;
    }

    if (_updateDeviceInProgress || _endpointRecoveryState == EndpointRecoveryState::Resolving
        || _endpointRecoveryState == EndpointRecoveryState::WaitingForRemoteAccessPrompt) {
        qCDebug(lcAccountState) << "Deferring pending endpoint recovery request because update flow is already active"
                                << "generation" << _pendingEndpointRecoveryRequest->generation
                                << "updateInProgress" << bool(_updateDeviceInProgress)
                                << "state" << static_cast<int>(_endpointRecoveryState);
        setEndpointRecoveryState(EndpointRecoveryState::Deferred);
        scheduleEndpointRecoveryRetry(1000);
        return;
    }

    if (_lastSyncTriggeredRecoveryAttempt.isValid()) {
        const auto elapsed = milliseconds(_lastSyncTriggeredRecoveryAttempt.elapsed());
        if (elapsed < syncTriggeredRecoveryCooldownInterval) {
            const auto remaining = duration_cast<milliseconds>(syncTriggeredRecoveryCooldownInterval - elapsed);
            qCDebug(lcAccountState) << "Sync-triggered endpoint recovery is cooling down"
                                    << "generation" << _pendingEndpointRecoveryRequest->generation
                                    << "remainingMs" << remaining.count();
            _syncTriggeredRecoveryCooldownTimer.start(int(remaining.count()));
            setEndpointRecoveryState(EndpointRecoveryState::Deferred);
            return;
        }
    }

    if (!device) {
        qCWarning(lcAccountState) << "Dropping pending endpoint recovery request without device"
                                  << "generation" << _pendingEndpointRecoveryRequest->generation;
        _pendingEndpointRecoveryRequest.reset();
        setEndpointRecoveryState(EndpointRecoveryState::Failed);
        return;
    }

    _syncTriggeredRecoveryCooldownTimer.stop();
    _endpointRecoveryRetryTimer.stop();
    _lastSyncTriggeredRecoveryAttempt.restart();
    setEndpointRecoveryState(EndpointRecoveryState::Resolving);
    qCInfo(lcAccountState).noquote()
        << "Starting immediate endpoint recovery"
        << QStringLiteral("generation=%1 folder=%2 reason=%3 activePathId=%4 baseUrl=%5")
               .arg(QString::number(pendingRequest.generation), pendingRequest.folderPath,
                    endpointRecoveryReasonString(pendingRequest.event.reason),
                    pendingRequest.event.activePathId ? pendingRequest.event.activePathId->toString(QUuid::WithoutBraces) : QStringLiteral("<none>"),
                    pendingRequest.event.baseUrl);
    setUpdateDeviceProgress(true);
    resolveAndApplyDevicePath(*device, true, DeviceUpdateTrigger::SyncTransportFailure);
}

void AccountState::scheduleNetworkTriggeredDeviceUpdate()
{
    if (_pendingEndpointRecoveryRequest) {
        qCDebug(lcAccountState) << "Skipping network-triggered device update scheduling because endpoint recovery is pending";
        if (_endpointRecoveryState == EndpointRecoveryState::Deferred) {
            scheduleEndpointRecoveryRetry(0);
        }
        return;
    }

    qCDebug(lcAccountState) << "Scheduling network-triggered device update";
    _networkChangeDebounceTimer.start(duration_cast<milliseconds>(networkChangeDebounceInterval).count());
}

void AccountState::runNetworkTriggeredDeviceUpdate()
{
    if (_pendingEndpointRecoveryRequest) {
        qCDebug(lcAccountState) << "Skipping network-triggered device update because endpoint recovery is pending";
        if (_endpointRecoveryState == EndpointRecoveryState::Deferred) {
            scheduleEndpointRecoveryRetry(0);
        }
        return;
    }

    if (!canStartDeviceAccessibilityUpdate(true)) {
        return;
    }

    if (_lastSuccessfulNetworkTriggeredDeviceUpdate.isValid()) {
        const auto elapsed = milliseconds(_lastSuccessfulNetworkTriggeredDeviceUpdate.elapsed());
        if (elapsed < networkChangeCooldownInterval) {
            const auto remaining = duration_cast<milliseconds>(networkChangeCooldownInterval - elapsed);
            qCDebug(lcAccountState) << "Network-triggered device update is cooling down for" << remaining.count() << "ms";
            _networkChangeDebounceTimer.start(int(remaining.count()));
            return;
        }
    }

    qCDebug(lcAccountState) << "Running network-triggered device update";
    setUpdateDeviceProgress(true);
    resolveAndApplyDevicePath(*_account->devicePtr(), true, DeviceUpdateTrigger::NetworkChange);
}

void AccountState::resolveAndApplyDevicePath(const Device& device, bool allowRemoteAccessPrompt, DeviceUpdateTrigger trigger)
{
    if (!_account) {
        qCWarning(lcAccountState) << "No account for device path resolution";
        setUpdateDeviceProgress(false);
        finishStartupDevicePathResolution(true);
        return;
    }

    if (!_deviceController) {
        qCWarning(lcAccountState) << "No device controller for device path resolution";
        setUpdateDeviceProgress(false);
        finishStartupDevicePathResolution(true);
        return;
    }

    const auto recoveryGeneration = (trigger == DeviceUpdateTrigger::SyncTransportFailure && _pendingEndpointRecoveryRequest)
        ? _pendingEndpointRecoveryRequest->generation
        : 0;
    const auto resolutionGeneration = ++_devicePathResolutionGeneration;
    qCInfo(lcAccountState) << "Starting device path resolution"
                           << "trigger" << deviceUpdateTriggerString(trigger)
                           << "generation" << recoveryGeneration
                           << "resolutionGeneration" << resolutionGeneration
                           << "allowRemoteAccessPrompt" << allowRemoteAccessPrompt
                           << "currentActivePath" << _account->activePath()
                           << "currentDavUrl" << _account->davUrl();

    _deviceController->resolveDevicePath(device)
        .then(this, [this, allowRemoteAccessPrompt, trigger, resolutionGeneration](const DevicePathResolutionResult& result) {
            if (resolutionGeneration != _devicePathResolutionGeneration || !_account || isSignedOut()) {
                qCDebug(lcAccountState) << "Ignoring stale device path resolution result"
                                        << "trigger" << deviceUpdateTriggerString(trigger)
                                        << "resultResolutionGeneration" << resolutionGeneration
                                        << "currentResolutionGeneration" << _devicePathResolutionGeneration;
                return;
            }
            const auto recoveryGeneration = (trigger == DeviceUpdateTrigger::SyncTransportFailure && _pendingEndpointRecoveryRequest)
                ? _pendingEndpointRecoveryRequest->generation
                : 0;
            qCInfo(lcAccountState) << "Device path resolution finished"
                                   << "trigger" << deviceUpdateTriggerString(trigger)
                                   << "generation" << recoveryGeneration
                                   << "resolutionGeneration" << resolutionGeneration
                                   << "outcome" << devicePathResolutionOutcomeString(result.outcome)
                                   << "resolved" << result.resolved()
                                   << "selectedPathId" << optionalUuidToString(result.selectedPathId)
                                   << "remoteAccessRequested" << result.remoteAccessRequested
                                   << "remoteCacheUpdated" << result.remoteCacheUpdated
                                   << "usedRemoteRelay" << result.usedRemoteRelay;
            if (allowRemoteAccessPrompt && result.outcome == DevicePathResolutionOutcome::RequiresRemoteAccessPrompt) {
                qCInfo(lcAccountState) << "Device path resolution requires Remote Access prompt"
                                       << "trigger" << deviceUpdateTriggerString(trigger)
                                       << "generation" << recoveryGeneration;
                requestRAupdate(result.device, trigger);
                return;
            }

            applyResolvedDevicePath(result, trigger);
        });
}

void AccountState::applyResolvedDevicePath(const DevicePathResolutionResult& result, DeviceUpdateTrigger trigger)
{
    const auto previousActivePathId = _account ? _account->activePath() : QUuid {};
    const auto previousDavUrl = _account ? _account->davUrl() : QUrl {};
    const auto recoveryGeneration = (trigger == DeviceUpdateTrigger::SyncTransportFailure && _pendingEndpointRecoveryRequest)
        ? _pendingEndpointRecoveryRequest->generation
        : 0;
    if (!result.resolved()) {
        qCWarning(lcAccountState) << "Device path resolution completed without reachable path"
                                  << "trigger" << deviceUpdateTriggerString(trigger)
                                  << "generation" << recoveryGeneration
                                  << "outcome" << devicePathResolutionOutcomeString(result.outcome)
                                  << "remoteAccessRequested" << result.remoteAccessRequested
                                  << "remoteCacheUpdated" << result.remoteCacheUpdated
                                  << "usedRemoteRelay" << result.usedRemoteRelay;
        if (trigger == DeviceUpdateTrigger::SyncTransportFailure && _pendingEndpointRecoveryRequest) {
            ++_endpointRecoveryGeneration;
            _pendingEndpointRecoveryRequest.reset();
            setEndpointRecoveryState(EndpointRecoveryState::Failed);
        }
        setUpdateDeviceProgress(false);
        finishStartupDevicePathResolution(true);
        return;
    }

    qCInfo(lcAccountState) << "Device path resolution selected reachable path"
                           << "trigger" << deviceUpdateTriggerString(trigger)
                           << "generation" << recoveryGeneration
                           << "selectedPathId" << result.selectedPathId.value()
                           << "outcome" << devicePathResolutionOutcomeString(result.outcome);
    _account->setResolvedDevice(result.device, result.selectedPathId.value());
    const auto selectedPathChanged = previousActivePathId != result.selectedPathId.value();
    const auto currentDavUrl = _account->davUrl();
    const auto davUrlChanged = previousDavUrl != currentDavUrl;
    if (trigger == DeviceUpdateTrigger::SyncTransportFailure && _pendingEndpointRecoveryRequest) {
        ++_endpointRecoveryGeneration;
        _pendingEndpointRecoveryRequest.reset();
        setEndpointRecoveryState(EndpointRecoveryState::Completed);
    }
    qCInfo(lcAccountState) << "Found path" << result.selectedPathId.value();
    if (trigger == DeviceUpdateTrigger::NetworkChange) {
        _lastSuccessfulNetworkTriggeredDeviceUpdate.restart();
    }
    if (selectedPathChanged || davUrlChanged) {
        qCInfo(lcAccountState) << "Applying resolved device path"
                               << "trigger" << deviceUpdateTriggerString(trigger)
                               << "generation" << recoveryGeneration
                               << "selectedPathId" << result.selectedPathId.value()
                               << "pathChanged" << selectedPathChanged
                               << "urlChanged" << davUrlChanged
                               << "newUrl" << currentDavUrl;
        finishStartupDevicePathResolution(false);
        _timeOfLastETagCheck = {};
        resetConnectionValidator();
        if (_state != Connected) {
            setState(Connecting);
        }
        emit urlChanged(_account->uuid());
        QTimer::singleShot(0, this, [this] {
            checkConnectivity(false);
        });
    } else {
        qCDebug(lcAccountState) << "Resolved device path keeps the same URL"
                                << "trigger" << deviceUpdateTriggerString(trigger)
                                << "generation" << recoveryGeneration
                                << "selectedPathId" << result.selectedPathId.value();
        finishStartupDevicePathResolution(false);
        if (_state != Connected) {
            _timeOfLastETagCheck = {};
            setState(Connecting);
            QTimer::singleShot(0, this, [this] {
                checkConnectivity(false);
            });
        }
    }
    setUpdateDeviceProgress(false);
}

void AccountState::requestRAupdate(const Device& device, DeviceUpdateTrigger trigger)
{
    if (device.certificateCommonName.isEmpty()) {
        qCWarning(lcAccountState) << "[requestRAupdate] No device for account";
        if (trigger == DeviceUpdateTrigger::SyncTransportFailure && _pendingEndpointRecoveryRequest) {
            ++_endpointRecoveryGeneration;
            _pendingEndpointRecoveryRequest.reset();
            setEndpointRecoveryState(EndpointRecoveryState::Failed);
        }
        setUpdateDeviceProgress(false);
        finishStartupDevicePathResolution(true);
        return;
    }

    if (!_deviceController) {
        qCWarning(lcAccountState) << "[requestRAupdate] No device controller";
        if (trigger == DeviceUpdateTrigger::SyncTransportFailure && _pendingEndpointRecoveryRequest) {
            ++_endpointRecoveryGeneration;
            _pendingEndpointRecoveryRequest.reset();
            setEndpointRecoveryState(EndpointRecoveryState::Failed);
        }
        setUpdateDeviceProgress(false);
        finishStartupDevicePathResolution(true);
        return;
    }

    qCDebug(lcAccountState) << "Requesting device update from RA for trigger" << deviceUpdateTriggerString(trigger);
    if (trigger == DeviceUpdateTrigger::SyncTransportFailure && _pendingEndpointRecoveryRequest) {
        setEndpointRecoveryState(EndpointRecoveryState::WaitingForRemoteAccessPrompt);
    }
    _remoteAccessPromptRetryTimer.stop();
    const auto generation = ++_devicePathUpdateGeneration;
    _activeAccessCodeGeneration = generation;
    _pendingDevicePathUpdate = PendingDevicePathUpdate {device, trigger, generation, false, false, true};
    qCInfo(lcAccountState) << "Starting Remote Access device update"
                           << "generation" << generation
                           << "trigger" << deviceUpdateTriggerString(trigger)
                           << "deviceCn" << device.certificateCommonName;

    connect(_deviceController, &DeviceController::account_update_device_finished, this, [this, generation](const Device& device) {
        if (!_pendingDevicePathUpdate || _pendingDevicePathUpdate->generation != generation) {
            qCDebug(lcAccountState) << "Ignoring stale device update result after RA flow was cancelled"
                                    << "resultGeneration" << generation
                                    << "currentGeneration" << (_pendingDevicePathUpdate ? _pendingDevicePathUpdate->generation : 0);
            return;
        }
        qCInfo(lcAccountState) << "Remote Access device update finished"
                               << "generation" << generation
                               << "trigger" << deviceUpdateTriggerString(_pendingDevicePathUpdate->trigger);
        emit pathUpdateFinished(false, device);
    }, Qt::SingleShotConnection);

    _deviceController->account_update_device(device);
}

void AccountState::tryShowRemoteAccessPrompt()
{
    if (!_pendingDevicePathUpdate || !_pendingDevicePathUpdate->awaitingAccessCode) {
        return;
    }

    if (!_account || isSignedOut()) {
        qCDebug(lcAccountState) << "Cancelling deferred RA prompt because account is unavailable"
                                << "generation" << (_pendingDevicePathUpdate ? _pendingDevicePathUpdate->generation : 0);
        _remoteAccessPromptRetryTimer.stop();
        _pendingDevicePathUpdate.reset();
        if (_pendingEndpointRecoveryRequest) {
            ++_endpointRecoveryGeneration;
            _pendingEndpointRecoveryRequest.reset();
            setEndpointRecoveryState(EndpointRecoveryState::Failed);
        }
        setUpdateDeviceProgress(false);
        return;
    }

    if (_tlsDialog || ocApp()->gui()->isAccountWizardActive()) {
        qCDebug(lcAccountState) << "Deferring RA prompt because another UI flow is active"
                                << "generation" << _pendingDevicePathUpdate->generation
                                << "trigger" << deviceUpdateTriggerString(_pendingDevicePathUpdate->trigger);
        _pendingDevicePathUpdate->accessCodePromptDeferred = true;
        _remoteAccessPromptRetryTimer.start(1000);
        if (_pendingDevicePathUpdate->trigger == DeviceUpdateTrigger::SyncTransportFailure && _pendingEndpointRecoveryRequest) {
            setEndpointRecoveryState(EndpointRecoveryState::Deferred);
        }
        return;
    }

    auto *settingsDialog = ocApp()->gui()->settingsDialog();
    if (!settingsDialog) {
        qCDebug(lcAccountState) << "Deferring RA prompt because settings dialog is unavailable"
                                << "generation" << _pendingDevicePathUpdate->generation
                                << "trigger" << deviceUpdateTriggerString(_pendingDevicePathUpdate->trigger);
        _pendingDevicePathUpdate->accessCodePromptDeferred = true;
        _remoteAccessPromptRetryTimer.start(1000);
        return;
    }

    auto overlay = settingsDialog->overlayController();
    if (!overlay) {
        qCDebug(lcAccountState) << "Deferring RA prompt because overlay controller is unavailable"
                                << "generation" << _pendingDevicePathUpdate->generation
                                << "trigger" << deviceUpdateTriggerString(_pendingDevicePathUpdate->trigger);
        _pendingDevicePathUpdate->accessCodePromptDeferred = true;
        _remoteAccessPromptRetryTimer.start(1000);
        return;
    }

    if (overlay->isBusy()) {
        qCDebug(lcAccountState) << "Deferring RA prompt because overlay controller is busy"
                                << "generation" << _pendingDevicePathUpdate->generation
                                << "trigger" << deviceUpdateTriggerString(_pendingDevicePathUpdate->trigger);
        _pendingDevicePathUpdate->accessCodePromptDeferred = true;
        _remoteAccessPromptRetryTimer.start(1000);
        if (_pendingDevicePathUpdate->trigger == DeviceUpdateTrigger::SyncTransportFailure && _pendingEndpointRecoveryRequest) {
            setEndpointRecoveryState(EndpointRecoveryState::Deferred);
        }
        return;
    }

    qCInfo(lcAccountState) << "Showing"
                           << (_pendingDevicePathUpdate->accessCodePromptDeferred ? "deferred" : "immediate")
                           << "RA prompt for generation" << _pendingDevicePathUpdate->generation
                           << "trigger" << deviceUpdateTriggerString(_pendingDevicePathUpdate->trigger);
    _remoteAccessPromptRetryTimer.stop();
    _pendingDevicePathUpdate->awaitingAccessCode = false;
    _pendingDevicePathUpdate->accessCodePromptDeferred = false;
    ApplicationGui::raise();
    overlay->requestAccessCode(_account->uuid(), _pendingDevicePathUpdate->clearAccessCodeOnPrompt);
    if (_pendingDevicePathUpdate->trigger == DeviceUpdateTrigger::SyncTransportFailure && _pendingEndpointRecoveryRequest) {
        setEndpointRecoveryState(EndpointRecoveryState::WaitingForRemoteAccessPrompt);
    }
}

std::optional<Device> AccountState::accountDevice() const
{
    if (_account)
        return _account->device();
    return std::nullopt;
}

void AccountState::initializeRA()
{
    if (_raInitialized) {
        qCDebug(lcAccountState) << "initializeRA already initialized";
        return;
    }

    qCDebug(lcAccountState) << "initializeRA";

    QPointer<OverlayController> oc = ocApp()->gui()->settingsDialog()->overlayController();
    if (!oc) {
        qCDebug(lcAccountState) << "invalid overlay controller";
        // TODO: Fatal?
    }

    connect(oc.get(), &OverlayController::codeEntered, this, [this](const QString &code, const QUuid& id) {
        if (_account && _account->uuid() == id && _pendingDevicePathUpdate
            && _pendingDevicePathUpdate->generation == _activeAccessCodeGeneration) {
            qCDebug(lcAccountState) << "codeEntered";
            _deviceController->enterAccessCode(code, false);
        } else { qCDebug(lcAccountState) << "codeEntered id isn't match"; }
    });

    connect(oc.get(), &OverlayController::resendRequested, this, [this](const QUuid& id) {
        if (_account && _account->uuid() == id) {
            qCDebug(lcAccountState) << "resendRequested";
            _deviceController->initAccessCode();
        } else { qCDebug(lcAccountState) << "resendRequested id isn't match"; }
    });

    connect(oc.get(), &OverlayController::processSkipped, this, [this,oc](const QUuid& id) {
        if (oc && _account && _account->uuid() == id) {
            qCDebug(lcAccountState) << "processSkipped";
            oc->hideAll();
            emit pathUpdateFinished(true, Device{});
        }
        else { qCDebug(lcAccountState) << "processSkipped id isn't match" << _account->uuid() << id; }
    });

    connect(oc.get(), &OverlayController::errorRetry, this, [this,oc](ErrorDialogState state, const QUuid& id) {
        if (oc && _account && _account->uuid() == id) {
            qCDebug(lcAccountState) << "errorRetry";
            if (state == ErrorDialogState::UnableToConnectToken) {
                oc->retryAccessCode(_account->uuid());
            }
        }
        else { qCDebug(lcAccountState) << "errorRetry id isn't match"; }
    });

    connect(oc.get(), &OverlayController::errorCancel, this, [this,oc](ErrorDialogState state, const QUuid& id) {
        if (_account && _account->uuid() == id) {
            qCDebug(lcAccountState) << "errorCancel";
            if (state == ErrorDialogState::UnableToConnectToken) {
                if (_pendingDevicePathUpdate) {
                    _pendingDevicePathUpdate->awaitingAccessCode = true;
                    _pendingDevicePathUpdate->clearAccessCodeOnPrompt = false;
                    tryShowRemoteAccessPrompt();
                }
            }
            else {
                emit pathUpdateFinished(true, Device{});
            }
        }
        else { qCDebug(lcAccountState) << "errorCancel id isn't match"; }
    });

    connect(oc.get(), &OverlayController::errorOk, this, [this](ErrorDialogState, const QUuid &id) {
        if (_account && _account->uuid() == id) {
            qCDebug(lcAccountState) << "errorOk";
            emit pathUpdateFinished(true, Device{});
        }
        else {
            qCDebug(lcAccountState) << "errorOk id isn't match";
        }
    });

    connect(_deviceController, &DeviceController::accessCodeRequest, this, [this,oc] {
        if (_account && !isSignedOut() && _pendingDevicePathUpdate) {
            qCDebug(lcAccountState) << "accessCodeRequest";
            _activeAccessCodeGeneration = _pendingDevicePathUpdate->generation;
            _pendingDevicePathUpdate->awaitingAccessCode = true;
            _pendingDevicePathUpdate->clearAccessCodeOnPrompt = true;
            tryShowRemoteAccessPrompt();
        }
        else { qCDebug(lcAccountState) << "overlay controller was destroyed or invalid account ptr"; }
    });

    connect(_deviceController, &DeviceController::accessCodeResult, this,
            [this,oc](DeviceController::AccessCodeContext context, int status_code, const QString &errorString, const QString &errorStacktrace) {
                if (isSignedOut() || !_pendingDevicePathUpdate
                    || _pendingDevicePathUpdate->generation != _activeAccessCodeGeneration) {
                    qCDebug(lcAccountState) << "Ignoring stale accessCodeResult after recovery flow was cancelled"
                                            << "pendingGeneration" << (_pendingDevicePathUpdate ? _pendingDevicePathUpdate->generation : 0)
                                            << "activeGeneration" << _activeAccessCodeGeneration;
                    if (oc && _account) {
                        oc->hideAll();
                    }
                    return;
                }
                if (status_code == 200) {
                    qCDebug(lcAccountState) << "accessCodeResult Accepted";
                    _pendingDevicePathUpdate->awaitingAccessCode = false;
                    _pendingDevicePathUpdate->accessCodePromptDeferred = false;
                    _deviceController->account_update_device_continue(_pendingDevicePathUpdate ? std::optional<Device>(_pendingDevicePathUpdate->device) : accountDevice());
                    oc->hideAll();
                } else {
                    qCDebug(lcAccountState) << "accessCodeResult Error"
                                            << (context == DeviceController::AccessCodeContext::Init ? "Init" : "Token")
                                            << status_code << errorString << errorStacktrace;
                    if (oc && _account) {
                        if (status_code == 401) {
                            // from /init - then incorrect email
                            // from /token - then invalid code
                            if (context == DeviceController::AccessCodeContext::Init) {
                                emit pathUpdateFinished(true, Device{});
                            }
                            else if (context == DeviceController::AccessCodeContext::Token) {
                                if (errorString.contains(QStringLiteral("expired"))) {
                                    oc->expiredAccessCode(_account->uuid());
                                }
                                else {
                                    oc->invalidAccessCode(_account->uuid());
                                }
                            }
                        }
                        else if (status_code == 500) {
                            if (context == DeviceController::AccessCodeContext::Token) {
                                oc->reportError(ErrorDialogState::UnableToConnectToken, _account->uuid());
                            }
                            else {
                                emit pathUpdateFinished(true, Device{});
                            }
                        }
                        else if (status_code == 429) {
                            if (context == DeviceController::AccessCodeContext::Init) {
                                //oc->reportError(ErrorDialogState::TooManyAttempts, _account->uuid());
                                oc->hideAll();
                                emit pathUpdateFinished(true, Device{});
                            }
                            else if (context == DeviceController::AccessCodeContext::Token) {
                                oc->resendAccessCode(_account->uuid());
                            }
                        }
                    }
                }
            });

    _raInitialized = true;
}

void AccountState::setUpdateDeviceProgress(bool inProgress)
{
    _updateDeviceInProgress = inProgress;
    qCDebug(lcAccountState) << "inProgress" << inProgress;
    if (!inProgress && _pendingEndpointRecoveryRequest && _endpointRecoveryState == EndpointRecoveryState::Deferred) {
        qCDebug(lcAccountState) << "Re-queueing deferred endpoint recovery after device update finished"
                                << "generation" << _pendingEndpointRecoveryRequest->generation
                                << "reason" << endpointRecoveryReasonString(_pendingEndpointRecoveryRequest->event.reason);
        _endpointRecoveryRetryTimer.stop();
        setEndpointRecoveryState(EndpointRecoveryState::Pending);
        QTimer::singleShot(0, this, &AccountState::triggerPendingEndpointRecovery);
    }
    emit networkUpdateState(inProgress);
}

void AccountState::slotConnectionValidatorResult(ConnectionValidator::Status status, const QStringList &errors)
{
    if (sender() != _connectionValidator) {
        qCDebug(lcAccountState) << "Ignoring stale connection validator result"
                                << "status" << status
                                << "sender" << sender()
                                << "currentValidator" << _connectionValidator.data();
        return;
    }

    if (isSignedOut()) {
        qCWarning(lcAccountState) << "Signed out, ignoring" << status << _account->url().toString();
        return;
    }

    if (status == ConnectionValidator::Connected && !_account->hasCapabilities()) {
        // this code should only be needed when upgrading from a < 3.0 release where capabilities where not cached
        // The last check was _waitingForNewCredentials = true so we only checked ValidateServer
        // now check again and fetch capabilities
        resetConnectionValidator();
        checkConnectivity();
        return;
    }

    // Come online gradually from 503 or maintenance mode
    if (status == ConnectionValidator::Connected
        && (_connectionStatus == ConnectionValidator::ServiceUnavailable
               || _connectionStatus == ConnectionValidator::MaintenanceMode)) {
        if (!_timeSinceMaintenanceOver.isValid()) {
            qCInfo(lcAccountState) << "AccountState reconnection: delaying for"
                                   << _maintenanceToConnectedDelay.count() << "ms";
            _timeSinceMaintenanceOver.start();
            QTimer::singleShot(_maintenanceToConnectedDelay + 100ms, this, [this] { AccountState::checkConnectivity(false); });
            return;
        } else if (_timeSinceMaintenanceOver.elapsed() < _maintenanceToConnectedDelay.count()) {
            qCInfo(lcAccountState) << "AccountState reconnection: only"
                                   << _timeSinceMaintenanceOver.elapsed() << "ms have passed";
            return;
        }
    }

    if (_connectionStatus != status) {
        qCInfo(lcAccountState) << "AccountState connection status change: "
                               << _connectionStatus << "->"
                               << status;
        _connectionStatus = status;
    }
    _connectionErrors = errors;

    if (Q_UNLIKELY(Theme::instance()->enableCernBranding())) {
        if (status == ConnectionValidator::Connected) {
            Q_ASSERT(_account->hasCapabilities());
            if (_account->capabilities().migration().space_migration.enabled) {
                auto statePtr = AccountManager::instance()->account(_account->uuid());
                auto migration = new SpaceMigration(statePtr, _account->capabilities().migration().space_migration.endpoint, this);
                connect(migration, &SpaceMigration::finished, this, [migration, this] {
                    migration->deleteLater();
                    setState(Connected);
                });
                migration->start();
                return;
            }
        }
    }
    switch (status) {
    case ConnectionValidator::Connected:
        setState(Connected);
        break;
    case ConnectionValidator::Undefined:
    case ConnectionValidator::NotConfigured:
        setState(Disconnected);
        break;
    case ConnectionValidator::ClientUnsupported:
        [[fallthrough]];
    case ConnectionValidator::ServerVersionMismatch:
        setState(ConfigurationError);
        break;
    case ConnectionValidator::StatusNotFound:
        // This can happen either because the server does not exist
        // or because we are having network issues. The latter one is
        // much more likely, so keep trying to connect.
        setState(NetworkError);
        break;
    case ConnectionValidator::CredentialsWrong:
    case ConnectionValidator::CredentialsNotReady:
        slotInvalidCredentials();
        break;
    case ConnectionValidator::SslError:
        // handled with the tlsDialog
        break;
    case ConnectionValidator::ServiceUnavailable:
        _timeSinceMaintenanceOver.invalidate();
        setState(ServiceUnavailable);
        break;
    case ConnectionValidator::MaintenanceMode:
        _timeSinceMaintenanceOver.invalidate();
        setState(MaintenanceMode);
        break;
    case ConnectionValidator::Timeout:
        setState(NetworkError);
        break;
    }
    resetConnectionValidator();
}

void AccountState::slotInvalidCredentials()
{
    if (!_waitingForNewCredentials) {
        qCInfo(lcAccountState) << "Invalid credentials for" << _account->url().toString();

        _waitingForNewCredentials = true;
        if (account()->credentials()->ready()) {
            account()->credentials()->invalidateToken();
        }
        if (auto creds = qobject_cast<HttpCredentials *>(account()->credentials())) {
            qCInfo(lcAccountState) << "refreshing oauth";
            if (creds->refreshAccessToken()) {
                return;
            }
            qCInfo(lcAccountState) << "refreshing oauth failed";
        }
        qCInfo(lcAccountState) << "asking user";
        account()->credentials()->askFromUser();
        setState(AskingCredentials);
    }
}

void AccountState::slotCredentialsFetched()
{
    // Make a connection attempt, no matter whether the credentials are
    // ready or not - we want to check whether we can get an SSL connection
    // going before bothering the user for a password.
    qCInfo(lcAccountState) << "Fetched credentials for" << _account->url().toString()
                           << "attempting to connect";
    _waitingForNewCredentials = false;
    scheduleEndpointRecoveryRetry(0);
    checkConnectivity();
}

void AccountState::slotCredentialsAsked()
{
    qCInfo(lcAccountState) << "Credentials asked for" << _account->url().toString() << "are they ready?" << _account->credentials()->ready();

    _waitingForNewCredentials = false;

    if (!_account->credentials()->ready()) {
        // User canceled the connection or did not give a password
        setState(SignedOut);
        return;
    }

    if (_connectionValidator) {
        // When new credentials become available we always want to restart the
        // connection validation, even if it's currently running.
        resetConnectionValidator();
    }

    scheduleEndpointRecoveryRetry(0);
    checkConnectivity();
}

std::unique_ptr<QSettings> AccountState::settings()
{
    auto s = ConfigFile::settingsWithGroup(QStringLiteral("Accounts"));
    s->beginGroup(_account->id());
    return s;
}

bool AccountState::supportsSpaces() const
{
    return _supportsSpaces && _account->hasCapabilities() && _account->capabilities().spacesSupport().enabled;
}

QuotaInfo *AccountState::quotaInfo()
{
    // QuotaInfo should not be used with spaces
    Q_ASSERT(!supportsSpaces());
    if (!_quotaInfo) {
        _quotaInfo = new QuotaInfo(this);
    }
    return _quotaInfo;
}

bool AccountState::isSettingUp() const
{
    return _settingUp;
}

void AccountState::setSettingUp(bool settingUp)
{
    if (_settingUp != settingUp) {
        _settingUp = settingUp;
        Q_EMIT isSettingUpChanged();
    }
}

void AccountState::createDeviceController()
{
    if (_deviceController) {
        qCWarning(lcAccountState) << "DeviceController already created";
        return;
    }

    _deviceController = new DeviceController(this);
    _deviceController->setEmail(_account->credentials()->user());

    if (Application::appCreated())
        initializeRA();
}

bool AccountState::readyForSync() const
{
    return !_fetchCapabilitiesJob && isConnected();
}

} // namespace APP
