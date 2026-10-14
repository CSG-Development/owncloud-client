/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "adminauthorization.h"

#include "globals.h"

#include <QDebug>
#include <QApplication>
#include <QMessageBox>
#include <QProcess>

#include <unistd.h>

#include <iostream>

#define PKEXEC_COMMAND "/usr/bin/pkexec"

namespace QInstaller {

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::AdminAuthorization
    \internal
*/

static void printError(QWidget *parent, const QString &value)
{
    if (qobject_cast<QApplication*> (qApp) != 0) {
        QMessageBox::critical(parent, QObject::tr( "Error acquiring admin rights" ), value,
            QMessageBox::Ok, QMessageBox::Ok);
    } else {
        std::cout << value.toStdString() << std::endl;
    }
}

bool AdminAuthorization::execute(QWidget *parent, const QString &program, const QStringList &arguments)
{
    const QString fallback = program + QLatin1String(" ") + arguments.join(QLatin1String(" "));
    qCDebug(QInstaller::lcServer) << "Fallback:" << fallback;

    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(QLatin1String(PKEXEC_COMMAND), QStringList() << program << arguments, QIODevice::ReadOnly);

    if (!process.waitForStarted() || !process.waitForFinished(-1)) {
        printError(parent, process.errorString());
        if (process.state() > QProcess::NotRunning)
            process.kill();
        return false;
    }
    if (process.exitCode() != EXIT_SUCCESS) {
        printError(parent, QLatin1String(process.readAll()));
        return false;
    }
    return true;
}

// has no guarantee to work
bool AdminAuthorization::hasAdminRights()
{
    return getuid() == 0;
}

} // namespace QInstaller
