/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include "kdtoolsglobal.h"
#include "filedownloaderfactory.h"
#include "abstractfiletask.h"
#include "packagemanagercore.h"

#include <QtCore/QObject>
#include <QtCore/QUrl>

#include <QtNetwork/QAuthenticator>
#include <QFutureWatcher>

using namespace QInstaller;

namespace KDUpdater {

class FileDownloaderProxyFactory;

class KDTOOLS_EXPORT FileDownloader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString scheme READ scheme WRITE setScheme)

public:
    explicit FileDownloader(const QString &scheme, QObject *parent = 0);
    ~FileDownloader();

    enum DownloadType {
        ChecksumFile,
        RegularFile
    };

    void resetFileItems();
    void addFileItem(FileTaskItem item);
    void addFileItems(QList<FileTaskItem> items);
    QList<FileTaskItem> fileItems() const;
    QList<FileTaskItem> fileItemsInChunks();

    QString scheme() const;
    void setScheme(const QString &scheme);

    void setPackageManagerCore(PackageManagerCore *core);

    virtual FileDownloader *clone(QObject *parent=0) const = 0;

    void download(DownloadType downloadType);
    template <typename AbstractTask>
    void setupFileTask(AbstractTask *const task, const DownloadType downloadType);
    void resetTasks();

    FileDownloaderProxyFactory *proxyFactory() const;
    void setProxyFactory(FileDownloaderProxyFactory *factory);

    quint64 bytesReceived() const;

    bool dataDownloded() const;
    void setDataDownloded(bool downloaded);
    bool sha1Downloded() const;
    void setSha1Downloded(bool downloaded);

    virtual void reset() = 0;

Q_SIGNALS:
    void downloadCompleted(const QString &sceme);
    void downloadAborted(const JobError error, const QString &errorStr);
    void registerFile(const QInstaller::FileTaskItem &item);
    void setProcessedAmount();
    void fileDownloaded(const QString &fileName, const QString &componentName);
    void networkDisconnected();
    void sha1DownloadFinished();
    void retryFileDownload(const QString &fileName);

public Q_SLOT:
    void setProgress(quint64 bytesReceived);

protected:
    void setDownloadCompleted();
    void setDownloadAborted(const JobError error, const QString &errorStr);
    bool isDownloadAborted() const;

private:
    QFutureWatcher<QInstaller::FileTaskResult> m_shaDownloadTask;
    QFutureWatcher<QInstaller::FileTaskResult> m_archiveDownloadTask;

private Q_SLOTS:
    virtual bool doDownload(DownloadType downloadType) = 0;
    void shaDownloadTaskFinished();
    void archiveDownloadTaskFinished();

private:
    struct Private;
    Private *d;
    QList<FileTaskResult> m_shaDownloadResult;
    QList<FileTaskResult> m_archiveDownloadResult;
    int m_downloadableChunkSize;
};

} // namespace KDUpdater

#endif // FILEDOWNLOADER_H
