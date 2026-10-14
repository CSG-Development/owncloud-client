/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef QINSTALLER_UTILS_H
#define QINSTALLER_UTILS_H

#include "installer_global.h"
#include "commandlineparser.h"

#include <QtCore/QBuffer>
#include <QtCore/QCryptographicHash>
#include <QtCore/QHash>
#include <QtCore/QUrl>
#include <QtCore/QTextStream>

#include <ostream>

QT_BEGIN_NAMESPACE
class QIODevice;
QT_END_NAMESPACE

namespace QInstaller {

    void INSTALLER_EXPORT uiDetachedWait(int ms);
    bool INSTALLER_EXPORT startDetached(const QString &program, const QStringList &arguments,
        const QString &workingDirectory, qint64 *pid = 0);

    QByteArray INSTALLER_EXPORT calculateHash(QIODevice *device, QCryptographicHash::Algorithm algo);
    QByteArray INSTALLER_EXPORT calculateHash(const QString &path, QCryptographicHash::Algorithm algo);

    QString INSTALLER_EXPORT replaceVariables(const QHash<QString,QString> &vars, const QString &str);
    QString INSTALLER_EXPORT replaceWindowsEnvironmentVariables(const QString &str);
    QStringList INSTALLER_EXPORT parseCommandLineArgs(int argc, char **argv);

    bool INSTALLER_EXPORT canCreateSymbolicLinks();

#ifdef Q_OS_WIN
    QString windowsErrorString(int errorCode);
    QString createCommandline(const QString &program, const QStringList &arguments);
    bool setPrivilege(const wchar_t *privilege, bool enable);
    bool checkPrivilege(const wchar_t *privilege);
    bool developerModeEnabled();
#endif

    QStringList INSTALLER_EXPORT localeCandidates(const QString &locale);

    QStringList INSTALLER_EXPORT checkMutualOptions(const CommandLineParser &parser, const QStringList &options);

    INSTALLER_EXPORT std::ostream& operator<<(std::ostream &os, const QString &string);
}

#endif // QINSTALLER_UTILS_H
