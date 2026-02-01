/*
 * Copyright (C) Fabian Müller <fmueller@owncloud.com>
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

#pragma once

#include "accountfwd.h"
#include "account.h"
#include "enums.h"
#include "gui/settingsdialog.h"
#include "setupcontext.h"
#include "setupwidget.h"
#include "pages/pagecontext.h"

#include <QDialog>

enum class ChangeReason {
    Default,
    EvaluationFailed,
};
Q_ENUM_NS(ChangeReason)

class DeviceController;

namespace CUR {
class DeviceListManager;
class EvaluatePath;
}

namespace CUR::Wizard {

enum class SetupResult {
    Success,
    Fail
};

/**
 * This class is the backbone of the new setup wizard. It instantiates the required UI elements and fills them with the correct data. It also provides the public API for the settings UI.
 *
 * The new setup wizard uses dependency injection where applicable. The account object is created using the builder pattern.
 */
class SetupController : public QObject
{
    Q_OBJECT

public:
    explicit SetupController(SettingsDialog *parent, RunAccountWizardReason reason);
    ~SetupController() noexcept override;

    SetupWidget *window();

    void setupFinishPage();

Q_SIGNALS:
    void setupFinishPageDefaults(const QString &defaultSyncTargetDir, const QString &userChosenSyncTargetDir, bool vfsIsAvailable, bool enableVfsByDefault, bool vfsModeIsExperimental);
    void finished(CUR::AccountPtr newAccount, CUR::Wizard::SyncMode syncMode, const QVariantMap &dynamicRegistrationData);
    void invalidServerUrl();

    // private, internal use
    void handleCredentialsEvaluation(CUR::Wizard::SetupResult result, const QString& msg = QString());
    void handleLoginResult(CUR::Wizard::SetupResult result, const QString& msg = QString());
    void handleFinishResult(CUR::Wizard::SetupResult result, const QString &msg = QString(), CUR::Wizard::SyncMode mode = SyncMode::Invalid);

    void cantFindDevice(QPrivateSignal);
    void evaluateCredentialsError(const QString& errStr, QPrivateSignal);

private:
    void onCredentialsAction(CredentialsAction action, std::optional<CredentialsContext> ctx = std::nullopt);

    void onLoginEmailClicked(const QString& email);
    void onDevicesUpdated(bool raQueried);

    void onFinishPageBackClicked();
    void onFinishPageDoneClicked(CUR::Wizard::SyncMode mode, const QString& targetDir);

    void onConnectErrorPageBackClicked();
    void onConnectErrorPageRetryClicked();

    void onInvalidServerUrl();

    void onHandleCredentialsEvaluation(SetupResult result, const QString& msg = QString());
    void onHandleLoginResult(SetupResult result, const QString& msg = QString());
    void onHandleFinishResult(SetupResult result, const QString &msg = QString(), SyncMode mode = SyncMode::Invalid);

    void onCantFindDevice();

    void evaluateCredentialsNew(const QUuid& id);
    void onEvaluateCredError(const QString& errStr);

    void performLogin();
    void evaluateFinishPage(CUR::Wizard::SyncMode mode, const QString& targetDir);

    SetupContext* _context = nullptr;
    // keeping a pointer on the current page allows us to check whether the controller has been initialized yet
    // the pointer is also used to clean up the page
    // QPointer<AbstractState> _currentState = nullptr;
    QString email_;
    QString password_;
    Device device_;
    QList<Device> fullList;
    DeviceController* _deviceController = nullptr;
    RunAccountWizardReason reason_ = RunAccountWizardReason::ApplicationStartup;
};
}
