/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef BINARYFORMATENGINE_H
#define BINARYFORMATENGINE_H

#include "binaryformat.h"

#include <QtCore/private/qfsfileengine_p.h>

namespace QInstaller {

class BinaryFormatEngine : public QAbstractFileEngine
{
    Q_DISABLE_COPY(BinaryFormatEngine)

public:
    BinaryFormatEngine(const QHash<QByteArray, ResourceCollection> &collections,
        const QString &fileName);

    void setFileName(const QString &file) override;

    bool copy(const QString &newName) override;
    bool close() override;
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
    bool open(QIODevice::OpenMode mode) override;
#else
    bool open(QIODevice::OpenMode mode,
              std::optional<QFile::Permissions> permissions = std::nullopt) override;
#endif
    qint64 pos() const override;
    qint64 read(char *data, qint64 maxlen) override;
    bool seek(qint64 offset) override;
    qint64 size() const override;

    QString fileName(FileName file = DefaultName) const override;
    FileFlags fileFlags(FileFlags type = FileInfoAll) const override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    IteratorUniquePtr beginEntryList(const QString &path, QDirListing::IteratorFlags filters, const QStringList &filterNames) override;
    QStringList entryList(QDirListing::IteratorFlags filters, const QStringList &filterNames) const;
#else
    Iterator *beginEntryList(QDir::Filters filters, const QStringList &filterNames) override;
#endif
    QStringList entryList(QDir::Filters filters, const QStringList &filterNames) const;

private:
    QStringList filterResults(QStringList result, const QStringList &filterNames) const;

private:
    QString m_fileNamePath;

    ResourceCollection m_collection;
    QSharedPointer<Resource> m_resource;

    QHash<QByteArray, ResourceCollection> m_collections;
};

} // namespace QInstaller

#endif // BINARYFORMATENGINE_H
