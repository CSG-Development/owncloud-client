/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "consumeoutputoperation.h"
#include "packagemanagercore.h"
#include "utils.h"
#include "globals.h"

#include <QFile>
#include <QDir>
#include <QProcess>
#include <QDebug>

using namespace QInstaller;

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::ConsumeOutputOperation
    \internal
*/

ConsumeOutputOperation::ConsumeOutputOperation(PackageManagerCore *core)
    : UpdateOperation(core)
{
    setName(QLatin1String("ConsumeOutput"));
}

void ConsumeOutputOperation::backup()
{
}

bool ConsumeOutputOperation::performOperation()
{
    // Arguments:
    // 1. key where the output will be saved
    // 2. executable path
    // 3. argument for the executable
    // 4. more arguments possible ...

    if (!checkArgumentCount(2, INT_MAX, tr("<to be saved installer key name> "
                                           "<executable> [argument1] [argument2] [...]")))
        return false;

    PackageManagerCore *const core = packageManager();
    if (!core) {
        setError(UserDefinedError);
        setErrorString(tr("Needed installer object in %1 operation is empty.").arg(name()));
        return false;
    }

    const QString installerKeyName = arguments().at(0);
    if (installerKeyName.isEmpty()) {
        setError(UserDefinedError);
        setErrorString(tr("Cannot save the output of \"%1\" to an empty installer key value.").arg(
            QDir::toNativeSeparators(arguments().at(1))));
        return false;
    }

    QString executable = arguments().at(1);

    QByteArray executableOutput;


    const QStringList processArguments = arguments().mid(2);
    // in some cases it is not runable, because another process is blocking it(filewatcher ...)
    int waitCount = 0;
    while (executableOutput.isEmpty() && waitCount < 3) {
        QProcess process;
        process.start(executable, processArguments, QIODevice::ReadOnly);
        if (process.waitForFinished(10000)) {
            if (process.exitStatus() == QProcess::CrashExit) {
                qCWarning(QInstaller::lcInstallerInstallLog) << executable
                    << processArguments << "crashed with exit code"
                    << process.exitCode() << "standard output: "
                    << process.readAllStandardOutput() << "error output: "
                    << process.readAllStandardError();
                setError(UserDefinedError);
                setErrorString(tr("Failed to run command: \"%1\": %2").arg(
                    QDir::toNativeSeparators(executable), process.errorString()));
                return false;
            }
            executableOutput.append(process.readAllStandardOutput());
        }
        if (executableOutput.isEmpty()) {
            ++waitCount;
            static const int waitTimeInMilliSeconds = 500;
            uiDetachedWait(waitTimeInMilliSeconds);
        }
        if (process.state() > QProcess::NotRunning ) {
            qCWarning(QInstaller::lcInstallerInstallLog) << executable
                << "process is still running, need to kill it.";
            process.kill();
        }

    }
    if (executableOutput.isEmpty()) {
        qCWarning(QInstaller::lcInstallerInstallLog) << "Cannot get any query output from executable"
            << executable;
    }
    core->setValue(installerKeyName, QString::fromLocal8Bit(executableOutput.trimmed()));
    return true;
}

bool ConsumeOutputOperation::undoOperation()
{
    return true;
}

bool ConsumeOutputOperation::testOperation()
{
    return true;
}
