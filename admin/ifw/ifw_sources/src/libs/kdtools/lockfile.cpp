/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "lockfile.h"
#include "lockfile_p.h"

namespace KDUpdater {

LockFile::LockFile(const QString &name)
    : d(new Private(name))
{
}

LockFile::~LockFile()
{
    delete d;
}

bool LockFile::lock()
{
    return d->lock();
}

QString LockFile::errorString() const
{
    return d->errorString;
}

bool LockFile::unlock()
{
    return d->unlock();
}

} // namespace KDUpdater
