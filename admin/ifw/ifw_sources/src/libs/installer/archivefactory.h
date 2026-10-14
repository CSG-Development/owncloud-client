/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef ARCHIVEFACTORY_H
#define ARCHIVEFACTORY_H

#include "installer_global.h"
#include "genericfactory.h"
#include "abstractarchive.h"

namespace QInstaller {

class INSTALLER_EXPORT ArchiveFactory
    : public GenericFactory<AbstractArchive, QString, QString, QObject *>
{
    Q_DISABLE_COPY(ArchiveFactory)

public:
    static ArchiveFactory &instance();

    template <typename T>
    void registerArchive(const QString &name, const QStringList &types)
    {
        if (containsProduct(name))
            m_supportedTypesHash.remove(name);

        registerProduct<T>(name);
        m_supportedTypesHash.insert(name, types);
    }
    AbstractArchive *create(const QString &filename, QObject *parent = nullptr) const;

    static QStringList supportedTypes();
    static bool isSupportedType(const QString &filename);

private:
    ArchiveFactory();

private:
    QHash<QString, QStringList> m_supportedTypesHash;
};

} // namespace QInstaller

#endif // ARCHIVEFACTORY_H
