/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef LIBARCHIVEWRAPPER_P_H
#define LIBARCHIVEWRAPPER_P_H

#include "installer_global.h"
#include "remoteobject.h"
#include "libarchivearchive.h"

#include <QTimer>
#include <QReadWriteLock>

namespace QInstaller {

class INSTALLER_EXPORT LibArchiveWrapperPrivate : public RemoteObject
{
    Q_OBJECT
    Q_DISABLE_COPY(LibArchiveWrapperPrivate)

public:
    explicit LibArchiveWrapperPrivate(const QString &filename);
    LibArchiveWrapperPrivate();
    ~LibArchiveWrapperPrivate();

    bool open(QIODevice::OpenMode mode);
    void close();
    void setFilename(const QString &filename);

    QString errorString() const;

    bool extract(const QString &dirPath, const quint64 totalFiles = 0);
    bool create(const QStringList &data);
    QVector<ArchiveEntry> list();
    bool isSupported();

    void setCompressionLevel(const AbstractArchive::CompressionLevel level);

Q_SIGNALS:
    void currentEntryChanged(const QString &filename);
    void completedChanged(const quint64 completed, const quint64 total);
    void dataBlockRequested();
    void seekRequested(qint64 offset, int whence);
    void remoteWorkerFinished();

public Q_SLOTS:
    void cancel();

private Q_SLOTS:
    void processSignals();
    void onDataBlockRequested();
    void onSeekRequested(qint64 offset, int whence);

private:
    void init();

    void addDataBlock(const QByteArray &buffer);
    void setClientDataAtEnd();
    void setClientFilePosition(qint64 pos);
    ExtractWorker::Status workerStatus() const;

private:
    mutable QReadWriteLock m_lock;

    LibArchiveArchive m_archive;
};

} // namespace QInstaller

#endif // LIBARCHIVEWRAPPER_P_H
