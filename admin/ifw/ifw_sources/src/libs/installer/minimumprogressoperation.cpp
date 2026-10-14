/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "minimumprogressoperation.h"

using namespace QInstaller;

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::MinimumProgressOperation
    \internal
*/

MinimumProgressOperation::MinimumProgressOperation(PackageManagerCore *core)
    : UpdateOperation(core)
{
    // This shouldn't be callable by script, but we need a name for the binary format
    setName(QLatin1String("MinimumProgress"));
}

void MinimumProgressOperation::backup()
{
}

bool MinimumProgressOperation::performOperation()
{
    emit progressChanged(1);
    return true;
}

bool MinimumProgressOperation::undoOperation()
{
    emit progressChanged(1);
    return true;
}

bool MinimumProgressOperation::testOperation()
{
    return true;
}
