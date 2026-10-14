/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "fakestopprocessforupdateoperation.h"

#include "messageboxhandler.h"
#include "packagemanagercore.h"

#include <QDir>

using namespace KDUpdater;
using namespace QInstaller;

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::FakeStopProcessForUpdateOperation
    \internal
*/

FakeStopProcessForUpdateOperation::FakeStopProcessForUpdateOperation(PackageManagerCore *core)
    : UpdateOperation(core)
{
    setName(QLatin1String("FakeStopProcessForUpdate"));
}

void FakeStopProcessForUpdateOperation::backup()
{
}

bool FakeStopProcessForUpdateOperation::performOperation()
{
    return true;
}

bool FakeStopProcessForUpdateOperation::undoOperation()
{
    setError(KDUpdater::UpdateOperation::NoError);
    if (!checkArgumentCount(1))
        return false;

    PackageManagerCore *const core = packageManager();
    if (!core) {
        setError(KDUpdater::UpdateOperation::UserDefinedError, tr("Cannot get package manager "
            "core."));
        return false;
    }

    QStringList processes = arguments().at(0).split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (int i = processes.count() - 1; i >= 0; --i) {
        if (!core->isProcessRunning(processes.at(i)))
            processes.removeAt(i);
    }

    if (processes.isEmpty())
        return true;

    if (processes.count() == 1) {
        setError(UpdateOperation::UserDefinedError, tr("This process should be stopped before "
            "continuing: %1").arg(QDir::toNativeSeparators(processes.first())));
    } else {
        const QString sep = QString::fromWCharArray(L"\n   \u2022 ");   // Unicode bullet
        setError(UpdateOperation::UserDefinedError, tr("These processes should be stopped before "
            "continuing: %1").arg(sep + QDir::toNativeSeparators(processes.join(sep))));
    }
    return false;
}

bool FakeStopProcessForUpdateOperation::testOperation()
{
    return true;
}
