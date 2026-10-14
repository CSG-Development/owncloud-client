/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef ABSTRACTARCHIVE_H
#define ABSTRACTARCHIVE_H

#include "installer_global.h"

#include <QFile>
#include <QDateTime>
#include <QDataStream>
#include <QPoint>

#ifdef Q_OS_WIN
typedef int mode_t;
#endif

namespace QInstaller {

struct INSTALLER_EXPORT ArchiveEntry
{
    ArchiveEntry()
        : isDirectory(false)
        , isSymbolicLink(false)
        , compressedSize(0)
        , uncompressedSize(0)
        , permissions_mode(0)
    {}

    ArchiveEntry(const ArchiveEntry &) = default;

    QString path;
    QDateTime utcTime;
    QPoint archiveIndex;
    bool isDirectory;
    bool isSymbolicLink;
    quint64 compressedSize;
    quint64 uncompressedSize;
    mode_t permissions_mode;
    QFile::Permissions permissions_enum;
};

class INSTALLER_EXPORT AbstractArchive : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AbstractArchive)

public:
    enum CompressionLevel {
        Non = 0,
        Fastest = 1,
        Fast = 3,
        Normal = 5,
        Maximum = 7,
        Ultra = 9
    };
    Q_ENUM(CompressionLevel)

    explicit AbstractArchive(QObject *parent = nullptr);
    virtual ~AbstractArchive() = 0;

    virtual bool open(QIODevice::OpenMode mode) = 0;
    virtual void close() = 0;
    virtual void setFilename(const QString &filename) = 0;

    virtual QString errorString() const;

    virtual bool extract(const QString &dirPath) = 0;
    virtual bool extract(const QString &dirPath, const quint64 totalFiles) = 0;
    virtual bool create(const QStringList &data) = 0;
    virtual QVector<ArchiveEntry> list() = 0;
    virtual bool isSupported() = 0;

    virtual void setCompressionLevel(const CompressionLevel level);

Q_SIGNALS:
    void currentEntryChanged(const QString &filename);
    void completedChanged(const quint64 completed, const quint64 total);

public Q_SLOTS:
    virtual void cancel() = 0;

protected:
    void setErrorString(const QString &error);
    CompressionLevel compressionLevel() const;

private:
    QString m_error;
    CompressionLevel m_compressionLevel;
};

INSTALLER_EXPORT QDataStream &operator>>(QDataStream &istream, ArchiveEntry &entry);
INSTALLER_EXPORT QDataStream &operator<<(QDataStream &ostream, const ArchiveEntry &entry);
INSTALLER_EXPORT bool operator==(const ArchiveEntry &lhs, const ArchiveEntry &rhs);

} // namespace QInstaller

#endif // ABSTRACTARCHIVE_H
