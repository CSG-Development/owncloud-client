/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef REPOSITORY_H
#define REPOSITORY_H

#include "installer_global.h"

#include <QtCore/QMetaType>
#include <QtCore/QUrl>

namespace QInstaller {

class INSTALLER_EXPORT Repository
{
public:
    explicit Repository();
    Repository(const Repository &other);
    explicit Repository(const QUrl &url, bool isDefault, bool compressed = false);

    static void registerMetaType();
    static Repository fromUserInput(const QString &repositoryUrl, bool compressed = false);

    bool isValid() const;
    bool isDefault() const;

    QUrl url() const;
    void setUrl(const QUrl &url);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    QString username() const;
    void setUsername(const QString &username);

    QString password() const;
    void setPassword(const QString &password);

    QString displayname() const;
    void setDisplayName(const QString &displayname);

    QString categoryname() const;
    void setCategoryName(const QString &categoryname);

    QByteArray xmlChecksum() const;
    void setXmlChecksum(const QByteArray &checksum);

    bool isCompressed() const;
    bool postLoadComponentScript() const;
    void setPostLoadComponentScript(const bool postLoad);

    bool operator==(const Repository &other) const;
    bool operator!=(const Repository &other) const;

    hashValue qHash(const Repository &repository);
    const Repository &operator=(const Repository &other);

    friend INSTALLER_EXPORT QDataStream &operator>>(QDataStream &istream, Repository &repository);
    friend INSTALLER_EXPORT QDataStream &operator<<(QDataStream &ostream, const Repository &repository);

private:
    QUrl m_url;
    bool m_default;
    bool m_enabled;
    QString m_username;
    QString m_password;
    QString m_displayname;
    QString m_categoryname;
    bool m_compressed;
    QByteArray m_xmlChecksum;
    bool m_postLoadComponentScript;
};

inline hashValue qHash(const Repository &repository)
{
    return qHash(repository.url());
}

INSTALLER_EXPORT QDataStream &operator>>(QDataStream &istream, Repository &repository);
INSTALLER_EXPORT QDataStream &operator<<(QDataStream &ostream, const Repository &repository);

} // namespace QInstaller

Q_DECLARE_METATYPE(QInstaller::Repository)

#endif // REPOSITORY_H
