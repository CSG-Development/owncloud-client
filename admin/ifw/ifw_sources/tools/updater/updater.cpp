/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include <errors.h>
#include <utils.h>
#include <constants.h>
#include <runoncechecker.h>
#include <qprocesswrapper.h>
#include <sysinfo.h>

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QThread>
#include <QDir>

using namespace QInstaller;
using namespace KDUpdater;

static void printUsageAndExit()
{
    qDebug() << "Usage: maintenanceToolUpdater --update-maintenancetool <newMtAppPath>,"
                "<existingMtAppPath>,<running mt pid>,<restart(optional)>";
    QCoreApplication::exit(EXIT_FAILURE);
}

static bool isProcessRunning(const QString &name, const quint32 pid)
{
    QList<ProcessInfo> processList = KDUpdater::runningProcesses();

    for (const ProcessInfo &processInfo : std::as_const(processList)) {
        if (processInfo.name.endsWith(name)) {
            if (pid != 0)
                return processInfo.id == pid;
            return true;
        }
    }
    return false;
}

static void callProcessWrapper(const QString &name, const QStringList &arguments)
{
    QProcessWrapper processWrapper;
    processWrapper.start(name, arguments);
    processWrapper.waitForFinished();
    if (!processWrapper.readAllStandardError().isEmpty()) {
        qDebug() << "Failed to perform operation " << name << " with arguments " << arguments;
        qDebug() << processWrapper.readAllStandardError();
        QCoreApplication::exit(EXIT_FAILURE);
    }
}

static bool removeMaintenanceTool(const QString &mtName)
{
    // Remove immutability and clear extended attribute from maintennacetool app bundle
    // otherwise the remove might fail in privileged folder
    QFileInfo fi(mtName);
    if (!fi.isBundle()) {
        qDebug() << mtName << " is not app bundle";
        return false;
    }

    qDebug() << "Remove maintenance tool " << mtName;
    callProcessWrapper(QLatin1String("chflags"), QStringList() << QLatin1String("-R") << QLatin1String("nouchg") << mtName);
    callProcessWrapper(QLatin1String("/bin/rm"), QStringList() << QLatin1String("-rf") << mtName);
    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    CommandLineParser parser;
    parser.parse(QInstaller::parseCommandLineArgs(argc, argv));

    if (!parser.isSet(CommandLineOptions::scUpdateMt))
        printUsageAndExit();

    QString newMt;
    QString existingMt;
    QString restart = scFalse;
    int pid = 0;

    QStringList arguments = parser.value(CommandLineOptions::scUpdateMt).split(QLatin1String(","));

    if (arguments.count() < 4)
        printUsageAndExit();

    newMt = arguments.at(0);
    existingMt = arguments.at(1);
    pid = arguments.at(2).toInt();
    restart = arguments.at(3);

    QTimer::singleShot(0, &app, [&]() {
        QDir mtBinaryPath(existingMt + QLatin1String("/Contents/MacOS"));
        QString executableName;
        QStringList entries = mtBinaryPath.entryList(QDir::Files | QDir::Executable | QDir::NoDotAndDotDot);
        if (!entries.isEmpty())
            executableName = entries.first();
        if (executableName.isEmpty()) {
            qDebug() << "No executable found inside macos bundle";
            return EXIT_FAILURE; // Should never happen
        }

        while (isProcessRunning(executableName, pid))
            QThread::sleep(1); // Wait until running maintenance tool finishes

        //Remove existing maintenance tool
        if (!removeMaintenanceTool(existingMt))
            return EXIT_FAILURE;

        // Copy new maintenance tool using ditto
        qDebug() << "Move " << newMt << " to " << existingMt;
        callProcessWrapper(QLatin1String("ditto"), QStringList() << newMt << existingMt);

        // Remove the tmp director where new maintenancetool exists
        QDir newMtDirectory(newMt);
        newMtDirectory.cdUp();
        QString mtDirectoryPath = QDir::cleanPath(newMtDirectory.absolutePath());

        QDir existingMtDirectory(existingMt);
        existingMtDirectory.cdUp();
        QString targetDirectoryPath = QDir::cleanPath(existingMtDirectory.absolutePath());

        //Make sure there is subdirectory in the targetdir so we don't delete the installation
        if (mtDirectoryPath.startsWith(targetDirectoryPath) && mtDirectoryPath != targetDirectoryPath) {
            removeMaintenanceTool(newMt);
            newMtDirectory.removeRecursively();
        }

        QStringList mtArguments;
        if (restart == scTrue) {
            // Restart with same command line arguments as first executable
            if (parser.isSet(CommandLineOptions::scMtArguments))
                mtArguments = parser.value(CommandLineOptions::scMtArguments).split(QLatin1String(" "));
            mtArguments.prepend(QCoreApplication::applicationFilePath());
            mtArguments.prepend(QLatin1String("--")
                        + CommandLineOptions::scCleanupUpdate);
        } else {
            mtArguments.append(QLatin1String("--")
                        + CommandLineOptions::scCleanupUpdateOnly);
            mtArguments.append(QCoreApplication::applicationFilePath());
        }

        // Relaunch updated app

        if (!QProcess::startDetached(existingMt, mtArguments)) {
            qWarning() << "Failed to relaunch updated app " << existingMt;
            QCoreApplication::exit(EXIT_FAILURE);
        }

        QCoreApplication::exit(EXIT_SUCCESS);
    });

    return app.exec();
}
