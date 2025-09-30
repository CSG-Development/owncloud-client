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

#include "accessmanager.h"
#include "setupaccountbuilder.h"

#include <QtGlobal>

namespace CUR {class SettingsDialog;}

namespace CUR::Wizard {

class SetupWidget;

/**
 * This class makes sharing wizard related objects between the controller and the states easier.
 * It also allows us to provide standardized
 */
class SetupContext : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(SetupContext)

public:
    explicit SetupContext(SettingsDialog* window, QObject *parent = nullptr);
    ~SetupContext() override;

    /**
     * Delete old access manager and create a new one.
     */
    AccessManager* resetAccessManager();

    SetupWidget *window() const;

    SetupAccountBuilder& accountBuilder();
    void resetAccountBuilder();

    AccessManager *accessManager() const;

    // convenience factory
    CoreJob *startFetchUserInfoJob(QObject *parent) const;

private:
    QPointer<SetupWidget> _window;
    AccessManager* _accessManager = nullptr;
    SetupAccountBuilder _accountBuilder;
};

} // CUR::Wizard
