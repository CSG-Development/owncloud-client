/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef LOCKFILE_H
#define LOCKFILE_H

#include "kdtoolsglobal.h"

namespace KDUpdater {

class KDTOOLS_EXPORT LockFile
{
    Q_DISABLE_COPY(LockFile)

public:
    explicit LockFile(const QString &name);
    ~LockFile();

    QString errorString() const;

    bool lock();
    bool unlock();

private:
    class Private;
    Private *d;
};

} // namespace KDUpdater

#endif // LOCKFILE_H
