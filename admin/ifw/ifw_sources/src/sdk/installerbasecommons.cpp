/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#include "installerbasecommons.h"

#include <packagemanagercore.h>
#include <scriptengine.h>
#include <packagemanagerpagefactory.h>
#include <productkeycheck.h>
#include <settings.h>

using namespace QInstaller;


// -- InstallerGui

InstallerGui::InstallerGui(PackageManagerCore *core)
    : PackageManagerGui(core, nullptr)
{
    ProductKeyCheck *checker = ProductKeyCheck::instance();
    foreach (const int id, checker->registeredPages()) {
        PackageManagerPage *page = PackageManagerPageFactory::instance().create(id, core);
        Q_ASSERT_X(page, Q_FUNC_INFO, qPrintable(QString::fromLatin1("Page with %1 couldn't be "
            "constructed.").arg(id)));
        setPage(id, page);
    }

    setPage(PackageManagerCore::Introduction, new IntroductionPage(core));
    setPage(PackageManagerCore::TargetDirectory, new TargetDirectoryPage(core));
    setPage(PackageManagerCore::ComponentSelection, new ComponentSelectionPage(core));
    setPage(PackageManagerCore::LicenseCheck, new LicenseAgreementPage(core));
#ifdef Q_OS_WIN
    setPage(PackageManagerCore::StartMenuSelection, new StartMenuDirectoryPage(core));
#endif
    setPage(PackageManagerCore::ReadyForInstallation, new ReadyForInstallationPage(core));
    setPage(PackageManagerCore::PerformInstallation, new PerformInstallationPage(core));
    setPage(PackageManagerCore::InstallationFinished, new FinishedPage(core));

    foreach (const int id, pageIds()) {
        QWizardPage *wizardPage = page(id);
        packageManagerCore()->controlScriptEngine()->addToGlobalObject(wizardPage);
        packageManagerCore()->componentScriptEngine()->addToGlobalObject(wizardPage);
    }
}


// -- MaintenanceGui

MaintenanceGui::MaintenanceGui(PackageManagerCore *core)
    : PackageManagerGui(core, nullptr)
{
    ProductKeyCheck *checker = ProductKeyCheck::instance();
    foreach (const int id, checker->registeredPages()) {
        PackageManagerPage *page = PackageManagerPageFactory::instance().create(id, core);
        Q_ASSERT_X(page, Q_FUNC_INFO, qPrintable(QString::fromLatin1("Page with %1 couldn't be "
            "constructed.").arg(id)));
        setPage(id, page);
    }

    connect(core, &PackageManagerCore::installerBinaryMarkerChanged,
            this, &MaintenanceGui::updateRestartPage);

    if (!core->isOfflineOnly() || validRepositoriesAvailable()) {
        setPage(PackageManagerCore::Introduction, new IntroductionPage(core));
        setPage(PackageManagerCore::ComponentSelection, new ComponentSelectionPage(core));
        setPage(PackageManagerCore::LicenseCheck, new LicenseAgreementPage(core));
    } else {
        core->setUninstaller();
        core->setCompleteUninstallation(true);
    }

    setPage(PackageManagerCore::ReadyForInstallation, new ReadyForInstallationPage(core));
    setPage(PackageManagerCore::PerformInstallation, new PerformInstallationPage(core));
    setPage(PackageManagerCore::InstallationFinished, new FinishedPage(core));

    RestartPage *p = new RestartPage(core);
    connect(p, &RestartPage::restart, this, &PackageManagerGui::gotRestarted);
    setPage(PackageManagerCore::InstallationFinished + 1, p);

    if (core->isUninstaller())
        wizardPageVisibilityChangeRequested(false, PackageManagerCore::InstallationFinished + 1);

    foreach (const int id, pageIds()) {
        QWizardPage *wizardPage = page(id);
        packageManagerCore()->controlScriptEngine()->addToGlobalObject(wizardPage);
        packageManagerCore()->componentScriptEngine()->addToGlobalObject(wizardPage);
    }
}

void MaintenanceGui::updateRestartPage()
{
    wizardPageVisibilityChangeRequested((packageManagerCore()->isUninstaller() ? false : true),
        PackageManagerCore::InstallationFinished + 1);
}

bool MaintenanceGui::validRepositoriesAvailable() const
{
    foreach (const Repository &repo, packageManagerCore()->settings().repositories()) {
        if (repo.isEnabled() && repo.isValid()) {
            return true;
        }
    }
    return false;
}
