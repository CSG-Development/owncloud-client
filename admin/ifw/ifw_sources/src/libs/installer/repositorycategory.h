/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef REPOSITORYCATEGORY_H
#define REPOSITORYCATEGORY_H

#include "installer_global.h"
#include "repository.h"
#include "qinstallerglobal.h"

#include <QtCore/QMetaType>
#include <QtCore/QUrl>
#include <QSet>

namespace QInstaller {

class INSTALLER_EXPORT RepositoryCategory
{

public:
    explicit RepositoryCategory();
    RepositoryCategory(const RepositoryCategory &other);

    static void registerMetaType();

    QString displayname() const;
    void setDisplayName(const QString &displayname);

    QString tooltip() const;
    void setTooltip(const QString &tooltip);

    QSet<Repository> repositories() const;
    void setRepositories(const QSet<Repository> repositories, const bool replace = false);
    void addRepository(const Repository &repository);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool operator==(const RepositoryCategory &other) const;
    bool operator!=(const RepositoryCategory &other) const;

    hashValue qHash(const RepositoryCategory &repository);

    friend INSTALLER_EXPORT QDataStream &operator>>(QDataStream &istream, RepositoryCategory &repository);
    friend INSTALLER_EXPORT QDataStream &operator<<(QDataStream &ostream, const RepositoryCategory &repository);

private:
    QMultiHash<QString, QVariant> m_data;
    QString m_displayname;
    QString m_tooltip;
    bool m_enabled;
};

inline hashValue qHash(const RepositoryCategory &repository)
{
    return qHash(repository.displayname());
}

INSTALLER_EXPORT QDataStream &operator>>(QDataStream &istream, RepositoryCategory &repository);
INSTALLER_EXPORT QDataStream &operator<<(QDataStream &ostream, const RepositoryCategory &repository);

} // namespace QInstaller

Q_DECLARE_METATYPE(QInstaller::RepositoryCategory)

#endif // REPOSITORYCATEGORY_H
