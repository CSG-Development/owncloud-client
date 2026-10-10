/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef INSTALLERBASECOMMONS_H
#define INSTALLERBASECOMMONS_H

#include <packagemanagergui.h>


// -- InstallerGui

class InstallerGui : public QInstaller::PackageManagerGui
{
    Q_OBJECT

public:
    explicit InstallerGui(QInstaller::PackageManagerCore *core);
    ~InstallerGui() {}
};


// -- MaintenanceGui

class MaintenanceGui : public QInstaller::PackageManagerGui
{
    Q_OBJECT

public:
    explicit MaintenanceGui(QInstaller::PackageManagerCore *core);
    ~MaintenanceGui() {}

private Q_SLOTS:
    void updateRestartPage();

private:
    bool validRepositoriesAvailable() const;
};

#endif // INSTALLERBASECOMMONS_H
