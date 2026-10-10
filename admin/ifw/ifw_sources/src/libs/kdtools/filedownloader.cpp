/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "filedownloader_p.h"
#include "filedownloaderfactory.h"

#include "downloadfiletask.h"
#include "copyfiletask.h"
#include "qinstallerglobal.h"
#include "packagemanagerproxyfactory.h"

#include <QDialog>
#include <QDir>
#include <QFile>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkProxyFactory>
#include <QPointer>
#include <QUrl>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QThreadPool>
#include <QDebug>
#include <QSslError>
#include <QBasicTimer>
#include <QTimerEvent>
#include <QLoggingCategory>
#include <globals.h>
#include <QHostInfo>
#include <QFutureWatcher>

using namespace KDUpdater;
using namespace QInstaller;

static constexpr uint scMaxRetries = 3;

// -- KDUpdater::FileDownloader

/*!
    \inmodule kdupdater
    \class KDUpdater::FileDownloader
    \internal
*/


struct KDUpdater::FileDownloader::Private
{
    Private()
        : m_factory(0)
        , m_bytesReceived(0)
        , m_allBytesReceived(0)
        , m_dataDownloaded(false)
        , m_sha1Downloaded(false)
        , m_aborted(false)
        , m_retryCount(scMaxRetries)
    {
    }

    ~Private()
    {
        delete m_factory;
    }

    QString scheme;

    PackageManagerCore *m_core;
    FileDownloaderProxyFactory *m_factory;
    QList<FileTaskItem> m_packages;
    quint64 m_bytesReceived;
    quint64 m_allBytesReceived;
    bool m_dataDownloaded;
    bool m_sha1Downloaded;
    bool m_aborted;
    int m_retryCount;
};

KDUpdater::FileDownloader::FileDownloader(const QString &scheme, QObject *parent)
    : QObject(parent)
    , d(new Private)
    , m_downloadableChunkSize(100)
{
    QByteArray downloadableChunkSize = qgetenv("IFW_DOWNLOAD_SIZE");
    if (!downloadableChunkSize.isEmpty()) {
        int chunkSize = QString::fromLocal8Bit(downloadableChunkSize).toInt();
        if (chunkSize > 0)
            m_downloadableChunkSize = chunkSize;
    }
    d->scheme = scheme;
    connect(&m_shaDownloadTask, &QFutureWatcherBase::finished, this, &FileDownloader::shaDownloadTaskFinished);
    connect(&m_archiveDownloadTask, &QFutureWatcherBase::finished, this, &FileDownloader::archiveDownloadTaskFinished);

}

KDUpdater::FileDownloader::~FileDownloader()
{
    delete d;
}

void FileDownloader::setDownloadAborted(const JobError error, const QString &errorStr)
{
    d->m_aborted = true;
    emit downloadAborted(error, errorStr);
}

void KDUpdater::FileDownloader::setDownloadCompleted()
{
    setDataDownloded(true);
    emit downloadCompleted(scheme());
}

void KDUpdater::FileDownloader::resetFileItems()
{
    d->m_packages.clear();
}

void KDUpdater::FileDownloader::addFileItem(FileTaskItem item)
{
    d->m_packages.append(item);
}

void KDUpdater::FileDownloader::addFileItems(QList<FileTaskItem> items)
{
    d->m_packages = items;
}

QList<FileTaskItem> KDUpdater::FileDownloader::fileItems() const
{
    return d->m_packages;
}

QList<FileTaskItem> KDUpdater::FileDownloader::fileItemsInChunks()
{
    QList<FileTaskItem> taskItems = fileItems();
    int chunkSize = qMin(taskItems.length(), m_downloadableChunkSize);
    QList<FileTaskItem> tempPackages = taskItems.mid(0, chunkSize);
    if (tempPackages.length() > 0)
        addFileItems(taskItems.mid(chunkSize, taskItems.length()));
    return tempPackages;
}

QString KDUpdater::FileDownloader::scheme() const
{
    return d->scheme;
}

void KDUpdater::FileDownloader::setScheme(const QString &scheme)
{
    d->scheme = scheme;
}

void KDUpdater::FileDownloader::setPackageManagerCore(PackageManagerCore *core)
{
    d->m_core = core;
}

void KDUpdater::FileDownloader::download(DownloadType downloadType)
{
    QMetaObject::invokeMethod(this, "doDownload", Qt::QueuedConnection, Q_ARG(DownloadType, downloadType));
}

template <typename AbstractTask>
void KDUpdater::FileDownloader::setupFileTask(AbstractTask *const task, const DownloadType downloadType)
{
    setDataDownloded(false);
    if (downloadType == RegularFile)
        connect(task, &AbstractTask::progressChanged, this, &KDUpdater::FileDownloader::setProgress);
    connect(task, &AbstractTask::fileDownloaded, this, &KDUpdater::FileDownloader::fileDownloaded);
    task->setProgressValueInBytes(true);
    if (downloadType == DownloadType::ChecksumFile)
        m_shaDownloadTask.setFuture(QtConcurrent::run(&AbstractTask::doTask, task));
    else
        m_archiveDownloadTask.setFuture(QtConcurrent::run(&AbstractTask::doTask, task));
}

void KDUpdater::FileDownloader::resetTasks()
{
    try {
        m_shaDownloadTask.cancel();
        m_shaDownloadTask.waitForFinished();
        m_archiveDownloadTask.cancel();
        m_archiveDownloadTask.waitForFinished();
    } catch (...) {}
}

void KDUpdater::FileDownloader::setProgress(quint64 bytesReceived)
{
    d->m_bytesReceived = bytesReceived + d->m_allBytesReceived;
    emit setProcessedAmount();
}

bool KDUpdater::FileDownloader::isDownloadAborted() const
{
    return d->m_aborted;
}

bool KDUpdater::FileDownloader::dataDownloded() const
{
    return d->m_dataDownloaded;
}

void KDUpdater::FileDownloader::setDataDownloded(bool downloaded)
{
    d->m_dataDownloaded = downloaded;
}

bool KDUpdater::FileDownloader::sha1Downloded() const
{
    return d->m_sha1Downloaded;
}

void KDUpdater::FileDownloader::setSha1Downloded(bool downloaded)
{
    d->m_sha1Downloaded = downloaded;
}

void KDUpdater::FileDownloader::shaDownloadTaskFinished()
{
    try {
        m_shaDownloadTask.waitForFinished();
        m_shaDownloadResult.append(m_shaDownloadTask.future().results());

        if (!doDownload(DownloadType::ChecksumFile)) {
            resetFileItems();
            foreach (const FileTaskResult &result, m_shaDownloadResult) {
                FileTaskItem item = result.taskItem();
                QString sourceUrl = item.source().remove(QLatin1String(".sha1"));
                QString targetUrl = item.target().remove(QLatin1String(".sha1"));
                item.insert(TaskRole::SourceFile, sourceUrl);
                item.insert(TaskRole::TargetFile, targetUrl);
                addFileItem(item);
            }
            //SHA files has been downloaded, now setup the archives to download
            setSha1Downloded(true);
            emit sha1DownloadFinished();
            doDownload(DownloadType::RegularFile);
        }
    }  catch (const AuthenticationRequiredException &e) {
        if (e.type() == AuthenticationRequiredException::Type::Proxy) {
            qCWarning(QInstaller::lcInstallerInstallLog) << e.message();
            PackageManagerProxyFactory *factory = d->m_core->proxyFactory();
            if (factory->askProxyCredentials(e.proxy())) {
                d->m_core->setProxyFactory(factory);
                doDownload(DownloadType::ChecksumFile);
            } else {
                reset();
                setDownloadAborted(QInstaller::DownloadError, tr("Missing proxy credentials."));
            }
        }
    } catch (const TaskException &e) {
        setDownloadAborted(QInstaller::DownloadError, e.message());
    } catch (const QUnhandledException &e) {
        setDownloadAborted(QInstaller::DownloadError, QLatin1String(e.what()));
    } catch (...) {
        setDownloadAborted(QInstaller::DownloadError, tr("Unknown exception during download."));
    }
}

void KDUpdater::FileDownloader::archiveDownloadTaskFinished()
{
    try {
        m_archiveDownloadTask.waitForFinished();

        m_archiveDownloadResult.append(m_archiveDownloadTask.future().results());
        d->m_allBytesReceived = d->m_bytesReceived;
        QList<FileTaskItem> failedItems;
        if (!doDownload(DownloadType::RegularFile)) {
            foreach (const FileTaskResult &result, m_archiveDownloadResult) {
                FileTaskItem item = result.value(TaskRole::TaskItem).value<FileTaskItem>();
                QByteArray checksum = result.value(TaskRole::Checksum).toByteArray().toHex();
                const QByteArray &expectedChecksum = item.value(TaskRole::Checksum).toByteArray();

                if (expectedChecksum != checksum) {
                    QString sourceUrl = item.source().append(QLatin1String(".sha1"));
                    QString targetUrl = item.target().append(QLatin1String(".sha1"));
                    item.insert(TaskRole::SourceFile, sourceUrl);
                    item.insert(TaskRole::TargetFile, targetUrl);
                    failedItems.append(item);
                    qCWarning(QInstaller::lcInstallerInstallLog) << tr("Hash verification error while "
                        "downloading %1. This can be a temporary error, retrying download.\n\n"
                        "Expected: %2 \nDownloaded: %3").arg(item.source(),
                        QString::fromLatin1(expectedChecksum), QString::fromLatin1(checksum));
                } else {
                    emit registerFile(item);
                }
            }
        } else {
            // Wait for all items to be downloaded
            return;
        }
        if (failedItems.count() == 0) {
            setDownloadCompleted();
        } else {
            --d->m_retryCount;
            if (d->m_retryCount <= 0) {
                setDownloadAborted(QInstaller::DownloadError, tr("Cannot verify Hash"));
            } else {
                m_shaDownloadResult.clear();
                m_archiveDownloadResult.clear();
                resetFileItems();
                addFileItems(failedItems);
                for (const FileTaskItem &item : std::as_const(failedItems)) {
                    QFileInfo fi(item.target());
                    emit retryFileDownload(fi.fileName());
                }
                doDownload(DownloadType::ChecksumFile);
            }
        }
    }  catch (const TaskException &e) {
        setDownloadAborted(QInstaller::DownloadError, e.message());
    } catch (const QUnhandledException &e) {
        setDownloadAborted(QInstaller::DownloadError, QLatin1String(e.what()));
    } catch (...) {
        setDownloadAborted(QInstaller::DownloadError, tr("Unknown exception during download."));
    }
}

FileDownloaderProxyFactory *KDUpdater::FileDownloader::proxyFactory() const
{
    if (d->m_factory)
        return d->m_factory->clone();
    return 0;
}

void KDUpdater::FileDownloader::setProxyFactory(FileDownloaderProxyFactory *factory)
{
    delete d->m_factory;
    d->m_factory = factory;
}

quint64 FileDownloader::bytesReceived() const
{
    return d->m_bytesReceived;
}

// -- KDUpdater::LocalFileDownloader

/*!
    \inmodule kdupdater
    \class KDUpdater::LocalFileDownloader
    \brief The LocalFileDownloader class is used to copy files from the local
    file system.
    \internal
*/

KDUpdater::LocalFileDownloader::LocalFileDownloader(QObject *parent)
    : KDUpdater::FileDownloader(QLatin1String("file"), parent)
{
}

KDUpdater::LocalFileDownloader::~LocalFileDownloader()
{
}

bool KDUpdater::LocalFileDownloader::doDownload(DownloadType downloadType)
{
    if (isDownloadAborted())
        return false;

    const QList<FileTaskItem> fileItemChunk = fileItemsInChunks();
    if (fileItemChunk.length() > 0) {
        CopyFileTask *const downloadTask = new CopyFileTask(fileItemChunk);
        setupFileTask(downloadTask, downloadType);
        return true;
    }
    return false;
}

KDUpdater::LocalFileDownloader *KDUpdater::LocalFileDownloader::clone(QObject *parent) const
{
    return new LocalFileDownloader(parent);
}

void LocalFileDownloader::reset()
{
    resetTasks();
}

// -- KDUpdater::HttpDownloader

/*!
    \inmodule kdupdater
    \class KDUpdater::HttpDownloader
    \brief The HttpDownloader class is used to download files over FTP, HTTP, or HTTPS.
    \internal
*/

KDUpdater::HttpDownloader::HttpDownloader(QObject *parent)
    : KDUpdater::FileDownloader(QLatin1String("http"), parent)
{

}

KDUpdater::HttpDownloader::~HttpDownloader()
{
}

bool KDUpdater::HttpDownloader::doDownload(DownloadType downloadType)
{
    if (isDownloadAborted())
        return false;

    const QList<FileTaskItem> fileItemChunk = fileItemsInChunks();
    if (fileItemChunk.length() > 0) {
        DownloadFileTask *const downloadTask = new DownloadFileTask(fileItemChunk);
        connect(downloadTask, &DownloadFileTask::networkDisconnected, this, &KDUpdater::HttpDownloader::networkDisconnected);
        setupFileTask(downloadTask, downloadType);
        return true;
    }
    return false;
}

KDUpdater::HttpDownloader *KDUpdater::HttpDownloader::clone(QObject *parent) const
{
    return new HttpDownloader(parent);
}

void KDUpdater::HttpDownloader::reset()
{
    resetTasks();
}
