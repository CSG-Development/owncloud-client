/*
 * Copyright (C) Fabian Müller <fmueller@owncloud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include <QDir>

#include "accountconfiguredstate.h"
#include "gui/folderman.h"
#include "pages/accountconfiguredpage.h"
#include "theme.h"

namespace APP::Wizard {

AccountConfiguredState::AccountConfiguredState(SetupWizardContext *context)
    : AbstractState(context)
{
    // being pessimistic by default
    bool vfsIsAvailable = false;
    bool enableVfsByDefault = false;
    bool vfsModeIsExperimental = false;

    switch (VfsPluginManager::instance().bestAvailableVfsMode()) {
    case Vfs::WindowsCfApi:
        vfsIsAvailable = true;
        enableVfsByDefault = true;
        vfsModeIsExperimental = false;
        break;
    case Vfs::WithSuffix:
        // we ignore forceVirtualFilesOption if experimental features are disabled
        vfsIsAvailable = Theme::instance()->enableExperimentalFeatures();
        enableVfsByDefault = false;
        vfsModeIsExperimental = true;
        break;
    default:
        break;
    }

    const auto urlToSuggestSyncFolderFor = [this]() {
        const auto selectedInstance = _context->accountBuilder().webFingerSelectedInstance();

        if (!selectedInstance.isEmpty()) {
            return selectedInstance;
        }

        return _context->accountBuilder().serverUrl();
    }();

    QString defaultSyncTargetDir = FolderMan::suggestSyncFolder(urlToSuggestSyncFolderFor, _context->accountBuilder().displayName());
    QString syncTargetDir = _context->accountBuilder().syncTargetDir();

    if (syncTargetDir.isEmpty()) {
        syncTargetDir = defaultSyncTargetDir;
    }

    _page = new AccountConfiguredPage(defaultSyncTargetDir, syncTargetDir, vfsIsAvailable, enableVfsByDefault, vfsModeIsExperimental);
}

SetupWizardState AccountConfiguredState::state() const
{
    return SetupWizardState::AccountConfiguredState;
}

void AccountConfiguredState::evaluatePage()
{
    auto accountConfiguredSetupWizardPage = qobject_cast<AccountConfiguredPage *>(_page);
    Q_ASSERT(accountConfiguredSetupWizardPage != nullptr);

    if (accountConfiguredSetupWizardPage->syncMode() != Wizard::SyncMode::ConfigureUsingFolderWizard) {
        QString syncTargetDir = QDir::fromNativeSeparators(accountConfiguredSetupWizardPage->syncTargetDir());

        // make sure we remember it now so we can show it to the user again upon failures
        _context->accountBuilder().setSyncTargetDir(syncTargetDir);

        const QString errorMessageTemplate = tr("Invalid local download directory: %1");

        if (!QDir::isAbsolutePath(syncTargetDir)) {
            Q_EMIT evaluationFailed(errorMessageTemplate.arg(QStringLiteral("path must be absolute")));
            return;
        }

        QString invalidPathErrorMessage = FolderMan::checkPathValidityRecursive(syncTargetDir);
        if (!invalidPathErrorMessage.isEmpty()) {
            Q_EMIT evaluationFailed(errorMessageTemplate.arg(invalidPathErrorMessage));
            return;
        }
    }

    Q_EMIT evaluationSuccessful();
}

} // APP::Wizard
