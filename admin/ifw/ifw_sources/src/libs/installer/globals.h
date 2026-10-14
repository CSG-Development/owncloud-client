/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#ifndef GLOBALS_H
#define GLOBALS_H

#include "installer_global.h"

#include <QRegularExpression>
#include <QLoggingCategory>

namespace QInstaller {

INSTALLER_EXPORT Q_DECLARE_LOGGING_CATEGORY(lcServer)
INSTALLER_EXPORT Q_DECLARE_LOGGING_CATEGORY(lcInstallerInstallLog)

INSTALLER_EXPORT Q_DECLARE_LOGGING_CATEGORY(lcProgressIndicator)
INSTALLER_EXPORT Q_DECLARE_LOGGING_CATEGORY(lcDeveloperBuild)

QStringList INSTALLER_EXPORT loggingCategories();

QRegularExpression INSTALLER_EXPORT commaRegExp();
inline QStringList splitStringWithComma(const QString &value) {
    if (!value.isEmpty())
        return value.split(QInstaller::commaRegExp(), Qt::SkipEmptyParts);
    return QStringList();
}

QString htmlToString(const QString &html);
QString enumToString(const QMetaObject& metaObject, const char *enumerator, int key);

template <typename T, template<typename> typename C>
QSet<T> toQSet(const C<T> &container)
{
    return QSet<T>(container.begin(), container.end());
}

void askForCredentials(QString *username, QString *password, const QString &usernameTitle, const QString &passwordTitle);
bool containsReservedCharacters(QString *message, const QStringList &packageEntryList);

}   // QInstaller

#endif  // GLOBALS_H
