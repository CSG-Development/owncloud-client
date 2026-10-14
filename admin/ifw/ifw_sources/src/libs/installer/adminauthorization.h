/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef ADMINAUTHORIZATION_H
#define ADMINAUTHORIZATION_H

#include <QWidget>

namespace QInstaller {

class AdminAuthorization
{
public:
    static bool hasAdminRights();
    static bool execute(QWidget *parent, const QString &programs, const QStringList &arguments);
};

} // namespace QInstaller

#endif // ADMINAUTHORIZATION_H
