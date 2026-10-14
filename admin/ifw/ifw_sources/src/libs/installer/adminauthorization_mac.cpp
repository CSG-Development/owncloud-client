/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "adminauthorization.h"

#include <Security/Authorization.h>
#include <Security/AuthorizationTags.h>

#include <QStringList>
#include <QVector>

#include <unistd.h>

namespace QInstaller {

bool AdminAuthorization::execute(QWidget *, const QString &program, const QStringList &arguments)
{
    AuthorizationRef authorizationRef;
    OSStatus status = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment,
        kAuthorizationFlagDefaults, &authorizationRef);
    if (status != errAuthorizationSuccess)
        return false;

    AuthorizationItem item = { kAuthorizationRightExecute, 0, 0, 0 };
    AuthorizationRights rights = { 1, &item };
    const AuthorizationFlags flags = kAuthorizationFlagDefaults | kAuthorizationFlagInteractionAllowed
        | kAuthorizationFlagPreAuthorize | kAuthorizationFlagExtendRights;

    status = AuthorizationCopyRights(authorizationRef, &rights, kAuthorizationEmptyEnvironment,
        flags, 0);
    if (status != errAuthorizationSuccess)
        return false;

    QVector<char *> args;
    QVector<QByteArray> utf8Args;
    foreach (const QString &argument, arguments) {
        utf8Args.push_back(argument.toUtf8());
        args.push_back(utf8Args.last().data());
    }
    args.push_back(0);

    const QByteArray utf8Program = program.toUtf8();
    status = AuthorizationExecuteWithPrivileges(authorizationRef, utf8Program.data(),
        kAuthorizationFlagDefaults, args.data(), 0);

    AuthorizationFree(authorizationRef, kAuthorizationFlagDestroyRights);
    return status == errAuthorizationSuccess;
}

bool AdminAuthorization::hasAdminRights()
{
    return geteuid() == 0;
}

} // namespace QInstaller
