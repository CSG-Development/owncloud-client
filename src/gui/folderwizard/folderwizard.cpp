/*
 * Copyright (C) by Duncan Mac-Vicar P. <duncan@kde.org>
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

#include "folderwizard.h"
#include "folderwizardpage.h"

#include "folderwizardlocalpath.h"
#include "folderwizardremotepath.h"
#include "folderwizardselectivesync.h"

#include "spacespage.h"

#include "account.h"
#include "common/asserts.h"
#include "configfile.h"
#include "creds/abstractcredentials.h"
#include "gui/application.h"
#include "gui/askexperimentalvirtualfilesfeaturemessagebox.h"
#include "gui/guiutility.h"
#include "gui/settingsdialog.h"
#include "networkjobs.h"
#include "theme.h"

#include "gui/accountstate.h"
#include "gui/folderman.h"
#include "gui/selectivesyncwidget.h"
#include "gui/spaces/spacesmodel.h"
#include "gui/customdialogs/custommessagebox.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QStackedWidget>
#include <QUrl>
#include <QVBoxLayout>

#include <stdlib.h>

namespace APP {

Q_LOGGING_CATEGORY(lcFolderWizard, "gui.folderwizard", QtInfoMsg)

QString FolderWizard::formatWarnings(const QStringList &warnings, bool isError)
{
    QString ret;
    if (warnings.count() == 1) {
        ret = isError ? QCoreApplication::translate("FolderWizard", "<b>Error:</b> %1").arg(warnings.first()) : QCoreApplication::translate("FolderWizard", "<b>Warning:</b> %1").arg(warnings.first());
    } else if (warnings.count() > 1) {
        QStringList w2;
        for (const auto &warning : warnings) {
            w2.append(QStringLiteral("<li>%1</li>").arg(warning));
        }
        ret = isError ? QCoreApplication::translate("FolderWizard", "<b>Error:</b><ul>%1</ul>").arg(w2.join(QString()))
                      : QCoreApplication::translate("FolderWizard", "<b>Warning:</b><ul>%1</ul>").arg(w2.join(QString()));
    }

    return ret;
}

QString FolderWizard::defaultSyncRoot() const
{
    if (!_account->account()->hasDefaultSyncRoot()) {
        return FolderMan::suggestSyncFolder(_account->account()->url(), _account->account()->davDisplayName());
    } else {
        return _account->account()->defaultSyncRoot();
    }
}

FolderWizard::FolderWizard(const AccountStatePtr &account, QWidget *parent)
    : QWidget(parent)
    , _account(account)
    , _stack(new QStackedWidget(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_stack);

    if (account->supportsSpaces()) {
        _spacesPage = new SpacesPage(this);
        addPage(_spacesPage);
    }

    _sourcePage = new FolderWizardLocalPath(this);
    addPage(_sourcePage);

    // for now spaces are meant to be synced as a whole
    if (!_account->supportsSpaces() && !Theme::instance()->singleSyncFolder()) {
        _targetPage = new FolderWizardRemotePath(this);
        addPage(_targetPage);
    }

    _selectiveSyncPage = new FolderWizardSelectiveSync(this);
    addPage(_selectiveSyncPage);

    _pages.first()->initializePage();
    showPage(0);
}

void FolderWizard::addPage(FolderWizardPage *page)
{
    _pages.append(page);
    _stack->addWidget(page);
    connect(page, &FolderWizardPage::completeChanged, this, &FolderWizard::navigationChanged);
}

void FolderWizard::showPage(int index)
{
    _stack->setCurrentIndex(index);
    Q_EMIT navigationChanged();
}

FolderWizardPage *FolderWizard::currentPage() const
{
    return _pages.value(_stack->currentIndex());
}

bool FolderWizard::canGoBack() const
{
    return _stack->currentIndex() > 0;
}

bool FolderWizard::canGoNext() const
{
    const auto *page = currentPage();
    return page && page->isComplete();
}

bool FolderWizard::isLastPage() const
{
    return _stack->currentIndex() == _pages.size() - 1;
}

void FolderWizard::back()
{
    if (!canGoBack()) {
        return;
    }
    currentPage()->cleanupPage();
    showPage(_stack->currentIndex() - 1);
}

void FolderWizard::next()
{
    auto *page = currentPage();
    if (!page || !page->isComplete() || !page->validatePage()) {
        return;
    }
    if (isLastPage()) {
        Q_EMIT completed();
        return;
    }
    const int index = _stack->currentIndex() + 1;
    _pages.at(index)->initializePage();
    showPage(index);
}

const AccountStatePtr &FolderWizard::accountState() const
{
    return _account;
}

QString FolderWizard::initialLocalPath() const
{
    if (_account->supportsSpaces()) {
        return FolderMan::findGoodPathForNewSyncFolder(defaultSyncRoot(), _spacesPage->selectedSpaceData(Spaces::SpacesModel::Columns::Name).toString());
    }

    // Split default sync root:
    const QFileInfo path(defaultSyncRoot());
    return FolderMan::findGoodPathForNewSyncFolder(path.path(), path.fileName());
}

QString FolderWizard::remotePath() const
{
    return _targetPage ? _targetPage->targetPath() : QString();
}

uint32_t FolderWizard::priority() const
{
    if (_account->supportsSpaces()) {
        return _spacesPage->selectedSpaceData(Spaces::SpacesModel::Columns::Priority).toInt();
    };
    return 0;
}

QUrl FolderWizard::davUrl() const
{
    if (_account->supportsSpaces()) {
        auto url = _spacesPage->selectedSpaceData(Spaces::SpacesModel::Columns::WebDavUrl).toUrl();
        if (!url.path().endsWith(QLatin1Char('/'))) {
            url.setPath(url.path() + QLatin1Char('/'));
        }
        return url;
    }
    return _account->account()->davUrl();
}

QString FolderWizard::spaceId() const
{
    if (_account->supportsSpaces()) {
        return _spacesPage->selectedSpaceData(Spaces::SpacesModel::Columns::SpaceId).toString();
    }
    return {};
}

QString FolderWizard::displayName() const
{
    if (_account->supportsSpaces()) {
        return _spacesPage->selectedSpaceData(Spaces::SpacesModel::Columns::Name).toString();
    };
    return QString();
}

bool FolderWizard::useVirtualFiles() const
{
    const auto mode = VfsPluginManager::instance().bestAvailableVfsMode();
    const bool useVirtualFiles = (Theme::instance()->forceVirtualFilesOption() && mode == Vfs::WindowsCfApi) || (_selectiveSyncPage->useVirtualFiles());
    if (useVirtualFiles) {
        const auto availability = Vfs::checkAvailability(initialLocalPath(), mode);
        if (!availability) {
            auto msg = new CustomMessageBox(window());
            msg->setHeaderText(FolderWizard::tr("Virtual files are not available for the selected folder"))
                .setMessageText(availability.error())
                .setSingleButtonText(FolderWizard::tr("OK"))
                .setSingleButton(true)
                .setDeleteOnClose(true);
            msg->open();
            return false;
        }
    }
    return useVirtualFiles;
}

FolderWizard::Result FolderWizard::result()
{
    const QString localPath = _sourcePage->localPath();
    if (!_account->account()->hasDefaultSyncRoot()) {
        if (FileSystem::isChildPathOf(localPath, defaultSyncRoot())) {
            _account->account()->setDefaultSyncRoot(defaultSyncRoot());
            if (!QFileInfo::exists(defaultSyncRoot())) {
                OC_ASSERT(QDir().mkpath(defaultSyncRoot()));
            }
        }
    }

    return {
        davUrl(), //
        spaceId(), //
        localPath, //
        remotePath(), //
        displayName(), //
        useVirtualFiles(), //
        priority(), //
        _selectiveSyncPage->selectiveSyncBlackList() //
    };
}

} // end namespace
