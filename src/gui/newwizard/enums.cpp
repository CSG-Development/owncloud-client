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

// just a stub so the MOC file can be included somewhere
#include "moc_enums.cpp"
#include "theme.h"

#include <QApplication>

using namespace APP::Wizard;

template <>
QString APP::Utility::enumToDisplayName(SetupState state)
{
    switch (state) {
    case SetupState::CredentialsState:
        return QApplication::translate("SetupWizardState", "Login");

    case SetupState::WaitState:
        return QApplication::translate("SetupWizardState", "Welcome");

    case SetupState::AccountConfiguredState:
        return QApplication::translate("SetupWizardState", "Sync Options");
    default:
        Q_UNREACHABLE();
    }
}
