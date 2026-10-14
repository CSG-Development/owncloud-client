/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef LOCKFILE_P_H
#define LOCKFILE_P_H

#include "lockfile.h"

#include <QString>

#ifdef Q_OS_WIN
# include <qt_windows.h>
#endif

namespace KDUpdater {

class LockFile::Private
{
public:
    explicit Private(const QString& name)
        : filename(name)
        , handle(0)
        , locked(false)
    {}

    bool lock();
    bool unlock();

    QString errorString;

private:
    QString filename;
#ifdef Q_OS_WIN
    HANDLE handle;
#else
    int handle;
#endif
    bool locked;
};

} // namespace KDUpdater

#endif // LOCKFILE_P_H
