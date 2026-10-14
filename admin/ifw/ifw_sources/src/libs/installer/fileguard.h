/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef FILEGUARD_H
#define FILEGUARD_H

#include "qinstallerglobal.h"

#include <QMutex>

namespace QInstaller {

class INSTALLER_EXPORT FileGuard
{
public:
    FileGuard() = default;

    bool tryLock(const QString &path);
    void release(const QString &path);

    static FileGuard *globalObject();

private:
    QMutex m_mutex;
    QStringList m_paths;
};

class INSTALLER_EXPORT FileGuardLocker
{
public:
    explicit FileGuardLocker(const QString &path, FileGuard *guard);
    ~FileGuardLocker();

private:
    QString m_path;
    FileGuard *m_guard;
};

} // namespace QInstaller

#endif // FILEGUARD_H
