/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "binaryreplace.h"

#include <binarycontent.h>
#include <copyfiletask.h>
#include <downloadfiletask.h>
#include <errors.h>
#include <fileio.h>
#include <fileutils.h>
#include <archivefactory.h>

#include <QDir>
#include <QFutureWatcher>

#include <iostream>

BinaryReplace::BinaryReplace(const QInstaller::BinaryLayout &layout)
    : m_binaryLayout(layout)
{}

int BinaryReplace::replace(const QString &source, const QString &target)
{
    const QUrl url = QUrl::fromUserInput(source);
    QFutureWatcher<QInstaller::FileTaskResult> taskWatcher;
    if (url.isRelative() || url.isLocalFile()) {
        taskWatcher.setFuture(QtConcurrent::run(&QInstaller::CopyFileTask::doTask,
            new QInstaller::CopyFileTask(QInstaller::FileTaskItem(url.isRelative()
            ? QFileInfo(source).absoluteFilePath() : url.toLocalFile()))));
    } else {
        taskWatcher.setFuture(QtConcurrent::run(&QInstaller::DownloadFileTask::doTask,
            new QInstaller::DownloadFileTask(QInstaller::FileTaskItem(url.toString()))));
    }

    bool result = EXIT_FAILURE;
    try {
        taskWatcher.waitForFinished(); // throws on error
        const QFuture<QInstaller::FileTaskResult> future = taskWatcher.future();
        if (future.resultCount() <= 0)
            return result;

        QString newInstallerBasePath = future.result().target();
        QScopedPointer<QInstaller::AbstractArchive> archive(
            QInstaller::ArchiveFactory::instance().create(newInstallerBasePath));

        if (archive && archive->open(QIODevice::ReadOnly) && archive->isSupported()) {
            if (archive->extract(QDir::tempPath())) {
                const QVector<QInstaller::ArchiveEntry> files = archive->list();
                newInstallerBasePath = QDir::tempPath() + QLatin1Char('/') + files.value(0).path;
                result = EXIT_SUCCESS;
            } else {
                std::cerr << qPrintable(QString::fromLatin1("Error while extracting \"%1\": %2")
                    .arg(QDir::toNativeSeparators(newInstallerBasePath), archive->errorString())) << std::endl;
            }

            if (!QFile::remove(newInstallerBasePath)) {
                std::cerr << qPrintable(QString::fromLatin1("Cannot delete file \"%1\"")
                    .arg(QDir::toNativeSeparators(newInstallerBasePath))) << std::endl;
            }
            if (result != EXIT_SUCCESS)
                return result;
        }

        result = EXIT_FAILURE;
        try {
            QFile installerBaseNew(newInstallerBasePath);
#ifndef Q_OS_MACOS
            QFile installerBaseOld(target);
            QInstaller::openForAppend(&installerBaseNew);

            installerBaseNew.seek(installerBaseNew.size());
            if (m_binaryLayout.magicMarker == QInstaller::BinaryContent::MagicInstallerMarker) {
                QInstaller::openForRead(&installerBaseOld);
                installerBaseOld.seek(m_binaryLayout.metaResourcesSegment.start());
                QInstaller::appendData(&installerBaseNew, &installerBaseOld, installerBaseOld
                    .size() - installerBaseOld.pos());
                installerBaseOld.close();
            } else {
                QInstaller::appendInt64(&installerBaseNew, 0);
                QInstaller::appendInt64(&installerBaseNew, 4 * sizeof(qint64));
                QInstaller::appendInt64(&installerBaseNew,
                    QInstaller::BinaryContent::MagicUninstallerMarker);
                QInstaller::appendInt64(&installerBaseNew,
                    QInstaller::BinaryContent::MagicCookie);
            }
            installerBaseNew.close();
#else
            QString bundlePath;
            QInstaller::isInBundle(target, &bundlePath);
            QFile installerBaseOld(QDir(bundlePath).filePath(bundlePath
                + QLatin1String("/Contents/MacOS/") + QFileInfo(bundlePath).completeBaseName()));
#endif
            QFile backup(installerBaseOld.fileName() + QLatin1String(".bak"));
            if (backup.exists() && (!backup.remove())) {
                std::cerr << qPrintable(QString::fromLatin1("Cannot delete \"%1\": %2")
                    .arg(QDir::toNativeSeparators(backup.fileName()), backup.errorString())) << std::endl;
            }

            const QString oldBasePath = installerBaseOld.fileName();
            if (!installerBaseOld.rename(oldBasePath + QLatin1String(".bak"))) {
                std::cerr << qPrintable(QString::fromLatin1("Cannot rename \"%1\" to \"%2\": %3")
                    .arg(oldBasePath, oldBasePath + QLatin1String(".bak"),
                    installerBaseOld.errorString())) << std::endl;
            }

            if (!installerBaseNew.rename(oldBasePath)) {
                std::cerr << qPrintable(QString::fromLatin1("Cannot copy \"%1\" to \"%2\": %3")
                    .arg(installerBaseNew.fileName(), oldBasePath, installerBaseNew.errorString()))
                    << std::endl;
            } else {
                result = EXIT_SUCCESS;
                installerBaseNew.setPermissions(installerBaseOld.permissions());
            }
        } catch (const QInstaller::Error &error) {
            std::cerr << qPrintable(error.message()) << std::endl;
        }
    } catch (const QInstaller::TaskException &e) {
        std::cerr << qPrintable(e.message()) << std::endl;
    } catch (const QUnhandledException &e) {
        std::cerr << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception caught."<< std::endl;
    }
    return result;
}
