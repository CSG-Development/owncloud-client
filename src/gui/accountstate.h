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


#ifndef ACCOUNTINFO_H
#define ACCOUNTINFO_H

#include "jobqueue.h"

#include "connectionvalidator.h"
#include "creds/abstractcredentials.h"
#include "device/devicecontroller.h"
#include "libsync/endpointrecoveryevent.h"
#include "updateurldialog.h"

#include <QByteArray>
#include <QElapsedTimer>
#include <QPointer>
#include <QTimer>
#include <memory>
#include <optional>

class QDialog;
class QSettings;

namespace APP {

class AccountState;
class Account;
class QuotaInfo;
class TlsErrorDialog;
class FetchServerSettingsJob;

/**
 * @brief Extra info about an ownCloud server account.
 * @ingroup gui
 */
class AccountState : public QObject
{
    Q_OBJECT
    Q_PROPERTY(AccountPtr account MEMBER _account READ account)

public:
    enum State {
        /// Not even attempting to connect, most likely because the
        /// user explicitly signed out or cancelled a credential dialog.
        SignedOut,

        /// Account would like to be connected but hasn't heard back yet.
        Disconnected,

        /// The account is successfully talking to the server.
        Connected,

        /// There's a temporary problem with talking to the server,
        /// don't bother the user too much and try again.
        ServiceUnavailable,

        /// Similar to ServiceUnavailable, but we know the server is down
        /// for maintenance
        MaintenanceMode,

        /// Could not communicate with the server for some reason.
        /// We assume this may resolve itself over time and will try
        /// again automatically.
        NetworkError,

        /// Server configuration error. (For example: unsupported version)
        ConfigurationError,

        /// We are currently asking the user for credentials
        AskingCredentials,

        Connecting
    };
    Q_ENUM(State)

    /// The actual current connectivity status.
    typedef ConnectionValidator::Status ConnectionStatus;

    ~AccountState() override;

    /** Creates an account state from settings and an Account object.
     *
     * Use from AccountManager with a prepared QSettings object only.
     */
    static std::unique_ptr<AccountState> loadFromSettings(AccountPtr account, const QSettings &settings);

    static std::unique_ptr<AccountState> fromNewAccount(AccountPtr account);

    /** Writes account state information to settings.
     *
     * It does not write the Account data.
     */
    void writeToSettings(QSettings &settings) const;

    AccountPtr account() const;

    ConnectionStatus connectionStatus() const;
    QStringList connectionErrors() const;

    State state() const;

    bool isSignedOut() const;

    [[nodiscard]] bool readyForSync() const;

    /** A user-triggered sign out which disconnects, stops syncs
     * for the account and forgets the password. */
    void signOutByUi();

    /** Tries to connect from scratch.
     *
     * Does nothing for signed out accounts.
     * Connected accounts will be disconnected and try anew.
     * Disconnected accounts will go to checkConnectivity().
     *
     * Useful for when network settings (proxy) change.
     */
    void freshConnectionAttempt();

    /// Move from SignedOut state to Disconnected (attempting to connect)
    void signIn();

    bool isConnected() const;

    // weather the account was created after spaces where implemented
    bool supportsSpaces() const;

    QuotaInfo *quotaInfo();

    /** Returns a new settings object for this account, already in the right groups. */
    std::unique_ptr<QSettings> settings();

    /** Mark the timestamp when the last successful ETag check happened for
     *  this account.
     *  The checkConnectivity() method uses the timestamp to save a call to
     *  the server to validate the connection if the last successful etag job
     *  was not so long ago.
     */
    void tagLastSuccessfullETagRequest(const QDateTime &tp);
    UpdateUrlDialog *updateUrlDialog(const QUrl &newUrl);

    /***
     * The account is setup for the first time, this may take some time
     */
    bool isSettingUp() const;
    void setSettingUp(bool settingUp);

    void createDeviceController();

public slots:
    /// Triggers a ping to the server to update state and
    /// connection status and errors.
    /// verifyServerState indicates that we must check the server
    void checkConnectivity(bool verifyServerState = false);

    /// Triggers checks and update status for all device URLs
    void updateDeviceAccessibility();
    void handleEndpointRecoveryRequest(const EndpointRecoveryEvent& event, const QString& folderPath);

private:
    enum class DeviceUpdateTrigger {
        Default,
        NetworkChange,
        SyncTransportFailure
    };

    enum class EndpointRecoveryState {
        Idle,
        Pending,
        Resolving,
        WaitingForRemoteAccessPrompt,
        Deferred,
        Completed,
        Failed
    };

    struct PendingDevicePathUpdate {
        Device device;
        DeviceUpdateTrigger trigger = DeviceUpdateTrigger::Default;
        quint64 generation = 0;
        bool awaitingAccessCode = false;
        bool accessCodePromptDeferred = false;
        bool clearAccessCodeOnPrompt = true;
    };

    struct PendingEndpointRecoveryRequest {
        EndpointRecoveryEvent event;
        QString folderPath;
        quint64 generation = 0;
    };

    /// Use the account as parent
    explicit AccountState(AccountPtr account);

    void setState(State state);
    void setEndpointRecoveryState(EndpointRecoveryState state);
    static const char *deviceUpdateTriggerString(DeviceUpdateTrigger trigger);

    bool canStartDeviceAccessibilityUpdate(bool logReason) const;
    bool canCoordinateEndpointRecovery(bool logReason) const;
    void enqueueEndpointRecoveryRequest(const EndpointRecoveryEvent& event, const QString& folderPath);
    bool shouldReplacePendingEndpointRecoveryRequest(const EndpointRecoveryEvent& event) const;
    void scheduleEndpointRecoveryRetry(int delayMs);
    void triggerPendingEndpointRecovery();
    void scheduleNetworkTriggeredDeviceUpdate();
    void runNetworkTriggeredDeviceUpdate();
    bool canRunQueuedNetworkTriggeredDeviceUpdate() const;
    bool shouldResolveStartupDevicePath() const;
    void startStartupDevicePathResolution(bool blockJobs);
    void finishStartupDevicePathResolution(bool continueConnectivity);
    void resolveAndApplyDevicePath(const Device& device, bool allowRemoteAccessPrompt, DeviceUpdateTrigger trigger,
        const std::optional<QUuid>& avoidPathId = std::nullopt);
    void applyResolvedDevicePath(const DevicePathResolutionResult& result, DeviceUpdateTrigger trigger);
    void requestRAupdate(const Device& device, DeviceUpdateTrigger trigger);
    void tryShowRemoteAccessPrompt();
    std::optional<Device> accountDevice() const;
    void initializeRA();
    void resetConnectionValidator();
    void setUpdateDeviceProgress(bool inProgress);

signals:
    void stateChanged(State state);
    void isConnectedChanged();
    void urlUpdated();
    void urlChanged(const QUuid& id);
    void isSettingUpChanged();
    void networkUpdateState(bool inProgress);

    // internal signal to be able to finish processing device path update when "Skip" code dialog pressed
    void pathUpdateFinished(bool skippedCode, const Device& device);

protected Q_SLOTS:
    void slotConnectionValidatorResult(ConnectionValidator::Status status, const QStringList &errors);
    void slotInvalidCredentials();
    void slotCredentialsFetched();
    void slotCredentialsAsked();

private:
    AccountPtr _account;
    JobQueueGuard _queueGuard;
    State _state;
    ConnectionStatus _connectionStatus;
    QStringList _connectionErrors;
    bool _waitingForNewCredentials;
    QDateTime _timeOfLastETagCheck;
    QPointer<ConnectionValidator> _connectionValidator;
    QPointer<UpdateUrlDialog> _updateUrlDialog;
    QPointer<TlsErrorDialog> _tlsDialog;
    bool _supportsSpaces = true;
    bool _settingUp = false;
    DeviceController* _deviceController = nullptr;
    std::atomic_bool _updateDeviceInProgress {false};
    bool _raInitialized = false;
    std::optional<PendingDevicePathUpdate> _pendingDevicePathUpdate;
    quint64 _devicePathUpdateGeneration = 0;
    quint64 _devicePathResolutionGeneration = 0;
    quint64 _activeAccessCodeGeneration = 0;
    EndpointRecoveryState _endpointRecoveryState = EndpointRecoveryState::Idle;
    std::optional<PendingEndpointRecoveryRequest> _pendingEndpointRecoveryRequest;
    quint64 _endpointRecoveryGeneration = 0;
    bool _startupDevicePathResolutionAttempted = false;
    bool _startupDevicePathResolutionInProgress = false;
    bool _startupConnectivityCheckDeferred = false;
    bool _startupConnectivityCheckBlockJobs = false;
    bool _networkTriggeredDeviceUpdatePending = false;
    QTimer _networkChangeDebounceTimer;
    QTimer _syncTriggeredRecoveryCooldownTimer;
    QTimer _endpointRecoveryRetryTimer;
    QTimer _remoteAccessPromptRetryTimer;
    QElapsedTimer _lastSuccessfulNetworkTriggeredDeviceUpdate;
    QElapsedTimer _lastSyncTriggeredRecoveryAttempt;

    /**
     * Starts counting when the server starts being back up after 503 or
     * maintenance mode. The account will only become connected once this
     * timer exceeds the _maintenanceToConnectedDelay value.
     */
    QElapsedTimer _timeSinceMaintenanceOver;

    /**
     * Milliseconds for which to delay reconnection after 503/maintenance.
     */
    std::chrono::milliseconds _maintenanceToConnectedDelay;

    QuotaInfo *_quotaInfo = nullptr;

    QPointer<FetchServerSettingsJob> _fetchCapabilitiesJob;

    friend class SpaceMigration;
};
}

Q_DECLARE_METATYPE(APP::AccountState *)
Q_DECLARE_METATYPE(APP::AccountStatePtr)

#endif //ACCOUNTINFO_H
