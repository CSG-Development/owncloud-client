/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "stopprocessforupdateoperation.h"

#include "packagemanagercore.h"

namespace QInstaller {

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::StopProcessForUpdateOperation
    \internal
*/

StopProcessForUpdateOperation::StopProcessForUpdateOperation(PackageManagerCore *core)
    : UpdateOperation(core)
{
    setName(QLatin1String("StopProcessForUpdate"));
}

void StopProcessForUpdateOperation::backup()
{
    return;
}

bool StopProcessForUpdateOperation::performOperation()
{
    setError(KDUpdater::UpdateOperation::NoError);
    if (!checkArgumentCount(1, 1, QLatin1String("<comma separated list of executable paths>")))
        return false;

    PackageManagerCore *const core = packageManager();
    if (!core) {
        setError(KDUpdater::UpdateOperation::UserDefinedError);
        setErrorString(tr("Cannot get package manager core."));
        return false;
    }

    QStringList processes = arguments().at(0).split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (const QString &process : std::as_const(processes)) {
        if (!core->killProcess(process, 5000)) {
            setError(UserDefinedError);
            setErrorString(tr("Could not terminate process \"%1\"").arg(process));
            return false;
        }
    }

    return true;
}

bool StopProcessForUpdateOperation::undoOperation()
{
    return performOperation();
}

bool StopProcessForUpdateOperation::testOperation()
{
    return true;
}

} // namespace QInstaller
