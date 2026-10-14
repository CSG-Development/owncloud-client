/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef METADATA_H
#define METADATA_H

#include "installer_global.h"
#include "genericdatacache.h"
#include "repository.h"

#include <QDomDocument>

class QFile;

namespace QInstaller {

class INSTALLER_EXPORT Metadata : public CacheableItem
{
public:
    Metadata();
    explicit Metadata(const QString &path);
    ~Metadata() {}

    QByteArray checksum() const override;
    void setChecksum(const QByteArray &checksum);
    QDomDocument updatesDocument() const;

    bool isValid() const override;
    bool isActive() const override;
    bool obsoletes(CacheableItem *other) override;

    Repository repository() const;
    void setRepository(const Repository &repository);

    bool isAvailableFromDefaultRepository() const;
    void setAvailableFromDefaultRepository(bool defaultRepository);

    void setPersistentRepositoryPath(const QUrl &url);
    QString persistentRepositoryPath();

    bool containsRepositoryUpdates() const;

private:
    bool verifyMetaFiles(QFile *updateFile) const;

private:
    Repository m_repository;
    QString m_persistentRepositoryPath;
    mutable QByteArray m_checksum;

    bool m_fromDefaultRepository;
};

} // namespace QInstaller

#endif // METADATA_H
