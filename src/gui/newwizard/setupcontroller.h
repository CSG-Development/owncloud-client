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

#include <QDialog>

enum class ChangeReason {
    Default,
    EvaluationFailed,
};
Q_ENUM_NS(ChangeReason)

namespace CUR {
class RemoteConnector;
class DeviceListManager;
}

namespace CUR::Wizard {


/**
 * This class is the backbone of the new setup wizard. It instantiates the required UI elements and fills them with the correct data. It also provides the public API for the settings UI.
 *
 * The new setup wizard uses dependency injection where applicable. The account object is created using the builder pattern.
 */
class SetupController : public QObject
{
    Q_OBJECT

public:
    explicit SetupController(SettingsDialog *parent);
    ~SetupController() noexcept override;

    SetupWidget *window();

    void setupFinishPage();

Q_SIGNALS:
    void setupFinishPageDefaults(const QString &defaultSyncTargetDir, const QString &userChosenSyncTargetDir, bool vfsIsAvailable, bool enableVfsByDefault, bool vfsModeIsExperimental);
    void finished(AccountPtr newAccount, SyncMode syncMode, const QVariantMap &dynamicRegistrationData);
    void credentialsEvaluationFailed(const QString& msg);
    void invalidServerUrl();
    void credentialsEvaluationSuccessful();
    void loginFailed(const QString& msg);
    void loginSuccessful();
    void finishSuccessful(SyncMode mode);
    void finishFailed(const QString& msg);

private:
    void startLogin(const Device &dev, const QString& url, const QString& user, const QString& password);
    void evaluateCredentials(const Device &dev, const QString &url, const QString &login, const QString &password);
    void performLogin();
    void evaluateFinishPage(CUR::Wizard::SyncMode mode, const QString& targetDir);

    SetupContext* _context = nullptr;
    // keeping a pointer on the current page allows us to check whether the controller has been initialized yet
    // the pointer is also used to clean up the page
    // QPointer<AbstractState> _currentState = nullptr;
    QString url_;
    QString user_;
    QString password_;
    Device device_;
    RemoteConnector* raConnector = nullptr;
    DeviceListManager* deviceMgr = nullptr;
    bool remoteSkipped = false;

    QList<DeviceInfo> remoteDevices;

    QList<MdnsRecord> localDevices;
    QList<Device> fullList;
};
}
