/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef RUNONCECHECKER_H
#define RUNONCECHECKER_H

#include "lockfile.h"

#include <QString>
#include <QCoreApplication>

class KDTOOLS_EXPORT RunOnceChecker
{
    Q_DISABLE_COPY(RunOnceChecker)

public:
    enum struct ConditionFlag {
        Lockfile = 0x01,
        ProcessList = 0x02
    };
    Q_DECLARE_FLAGS(ConditionFlags, ConditionFlag)

    explicit RunOnceChecker(const QString &filename = QString());
    ~RunOnceChecker();

    bool isRunning(RunOnceChecker::ConditionFlags flags, const QString &appPath = QCoreApplication::applicationFilePath());

private:
    KDUpdater::LockFile m_lockfile;
};

#endif // RUNONCECHECKER_H
