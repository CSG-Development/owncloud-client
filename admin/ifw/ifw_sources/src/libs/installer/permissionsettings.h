/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef PERMISSIONSETTINGS_H
#define PERMISSIONSETTINGS_H

#include <QSettings>

namespace QInstaller {

class PermissionSettings : public QSettings
{
public:
    explicit PermissionSettings(const QString &organization,
        const QString &application = QString(), QObject *parent = 0)
        : QSettings(organization, application, parent) {}

    PermissionSettings(Scope scope, const QString &organization,
        const QString &application = QString(), QObject *parent = 0)
        : QSettings(scope, organization, application, parent) {}

    PermissionSettings(Format format, Scope scope, const QString &organization,
       const QString &application = QString(), QObject *parent = 0)
       : QSettings(format, scope, organization, application, parent) {}
    PermissionSettings(const QString &fileName, Format format, QObject *parent = 0)
       : QSettings(fileName, format, parent) {}

    ~PermissionSettings();
};

}

#endif  // PERMISSIONSETTINGS_H
