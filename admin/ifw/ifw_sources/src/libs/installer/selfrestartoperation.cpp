/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "selfrestartoperation.h"
#include "packagemanagercore.h"

#include "selfrestarter.h"

using namespace QInstaller;

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::SelfRestartOperation
    \internal
*/

SelfRestartOperation::SelfRestartOperation(PackageManagerCore *core)
    : UpdateOperation(core)
{
    setName(QLatin1String("SelfRestart"));
}

void SelfRestartOperation::backup()
{
    setValue(QLatin1String("PreviousSelfRestart"), SelfRestarter::restartOnQuit());
}

bool SelfRestartOperation::performOperation()
{
    PackageManagerCore *const core = packageManager();
    if (!core) {
        setError(UserDefinedError);
        setErrorString(tr("Installer object needed in operation %1 is empty.").arg(name()));
        return false;
    }

    if (!core->isMaintainer()) {
        setError(UserDefinedError);
        setErrorString(tr("Self Restart: Only valid within updater or package manager mode."));
        return false;
    }

    if (!arguments().isEmpty()) {
        setError(InvalidArguments);
        setErrorString(tr("Self Restart: Invalid arguments"));
        return false;
    }
    SelfRestarter::setRestartOnQuit(true);
    return SelfRestarter::restartOnQuit();
}

bool SelfRestartOperation::undoOperation()
{
    SelfRestarter::setRestartOnQuit(value(QLatin1String("PreviousSelfRestart")).toBool());
    return true;
}

bool SelfRestartOperation::testOperation()
{
    return true;
}
