/* Copyright C 2002 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef UPDATER_H
#define UPDATER_H

#include "kdtoolsglobal.h"

namespace KDUpdater
{
    enum Error
    {
        ENoError = 0,
        ECannotStartTask,
        ECannotPauseTask,
        ECannotResumeTask,
        ECannotStopTask,
        EUnknown
    };
    KDTOOLS_EXPORT int compareVersion(const QString &v1, const QString &v2);
}

#endif // UPDATER_H
