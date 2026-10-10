/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef LIBARCHIVEWRAPPER_H
#define LIBARCHIVEWRAPPER_H

#include "installer_global.h"
#include "abstractarchive.h"

namespace QInstaller {

class LibArchiveWrapperPrivate;

class INSTALLER_EXPORT LibArchiveWrapper : public AbstractArchive
{
    Q_OBJECT
    Q_DISABLE_COPY(LibArchiveWrapper)

public:
    LibArchiveWrapper(const QString &filename, QObject *parent = nullptr);
    explicit LibArchiveWrapper(QObject *parent = nullptr);
    ~LibArchiveWrapper();

    bool open(QIODevice::OpenMode mode) override;
    void close() override;
    void setFilename(const QString &filename) override;

    QString errorString() const override;

    bool extract(const QString &dirPath) override;
    bool extract(const QString &dirPath, const quint64 totalFiles) override;
    bool create(const QStringList &data) override;
    QVector<ArchiveEntry> list() override;
    bool isSupported() override;

    void setCompressionLevel(const AbstractArchive::CompressionLevel level) override;

public Q_SLOTS:
    void cancel() override;

private:
    LibArchiveWrapperPrivate *const d;
};

} // namespace QInstaller

#endif // LIBARCHIVEWRAPPER_H
