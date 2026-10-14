/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#include "permissionsettings.h"

#include <QFile>

using namespace QInstaller;

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::PermissionSettings
    \internal
*/

PermissionSettings::~PermissionSettings()
{
    if (!fileName().isEmpty()) {
        sync();
        QFile file(fileName());
        file.setPermissions(file.permissions() | QFile::ReadGroup | QFile::ReadOther);
    }
}

