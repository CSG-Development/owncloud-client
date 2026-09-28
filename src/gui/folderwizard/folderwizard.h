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

#pragma once

#include <QList>
#include <QLoggingCategory>
#include <QSet>
#include <QUrl>
#include <QWidget>

#include "accountfwd.h"

#include "gui/folder.h"

class QStackedWidget;

namespace APP {

Q_DECLARE_LOGGING_CATEGORY(lcFolderWizard)

class FolderWizardPage;
class SpacesPage;
class FolderWizardLocalPath;
class FolderWizardRemotePath;
class FolderWizardSelectiveSync;

/**
 * @brief The FolderWizard class
 * @ingroup gui
 */
class FolderWizard : public QWidget
{
    Q_OBJECT
public:
    struct Result
    {
        /***
         * The webdav url for the sync connection.
         */
        QUrl davUrl;

        /***
         * The id of the space or empty in case of ownCloud 10.
         */
        QString spaceId;

        /***
         * The local folder used for the sync.
         */
        QString localPath;

        /***
         * The relative remote path
         */
        QString remotePath;

        /***
         * The Space name to display in the list of folders or an empty string.
         */
        QString displayName;

        /***
         * Wether to use virtual files.
         */
        bool useVirtualFiles;

        uint32_t priority;

        QSet<QString> selectiveSyncBlackList;
    };

    explicit FolderWizard(const AccountStatePtr &account, QWidget *parent = nullptr);

    Result result();

    bool canGoBack() const;
    bool canGoNext() const;
    bool isLastPage() const;

    void back();
    void next();

    const AccountStatePtr &accountState() const;
    QString initialLocalPath() const;
    QString remotePath() const;
    uint32_t priority() const;
    QString defaultSyncRoot() const;
    QUrl davUrl() const;
    QString spaceId() const;
    bool useVirtualFiles() const;
    QString displayName() const;

    static QString formatWarnings(const QStringList &warnings, bool isError = false);

Q_SIGNALS:
    void navigationChanged();
    void completed();

private:
    void addPage(FolderWizardPage *page);
    void showPage(int index);
    FolderWizardPage *currentPage() const;

    AccountStatePtr _account;
    QStackedWidget *_stack = nullptr;
    QList<FolderWizardPage *> _pages;
    SpacesPage *_spacesPage = nullptr;
    FolderWizardLocalPath *_sourcePage = nullptr;
    FolderWizardRemotePath *_targetPage = nullptr;
    FolderWizardSelectiveSync *_selectiveSyncPage = nullptr;
};

} // namespace APP
