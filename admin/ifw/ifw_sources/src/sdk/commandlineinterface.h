/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef COMMANDLINEINTERFACE_H
#define COMMANDLINEINTERFACE_H

#include "sdkapp.h"

namespace QInstaller {
    class PackageManagerCore;
}

class CommandLineInterface : public SDKApp<QCoreApplication>
{
    Q_OBJECT
    Q_DISABLE_COPY(CommandLineInterface)

public:
    CommandLineInterface(int &argc, char *argv[]);
    ~CommandLineInterface();

public:
    int checkUpdates();
    int listInstalledPackages();
    int searchAvailablePackages();
    int updatePackages();
    int installPackages();
    int uninstallPackages();
    int removeInstallation();
    int createOfflineInstaller(const QInstaller::PackageManagerCore::HybridInstaller createHybrid);
    int clearLocalCache();

private:
    bool initialize();
    bool checkLicense();
    bool setTargetDir();
    QHash<QString, QString> parsePackageFilters();

    QStringList m_positionalArguments;
};

#endif // COMMANDLINEINTERFACE_H
