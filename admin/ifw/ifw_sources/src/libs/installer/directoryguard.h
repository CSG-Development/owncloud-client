/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef DIRECTORYGUARD_H
#define DIRECTORYGUARD_H

#include "installer_global.h"

#include <QString>

namespace QInstaller {

class INSTALLER_EXPORT DirectoryGuard
{
public:
    explicit DirectoryGuard(const QString &path);
    ~DirectoryGuard();

    QStringList tryCreate();
    void release();

private:
    QString m_path;
    bool m_created;
    bool m_released;
};

} // namespace QInstaller

#endif // DIRECTORYGUARD_H
