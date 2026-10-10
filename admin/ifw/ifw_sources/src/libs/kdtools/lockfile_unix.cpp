/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "lockfile_p.h"

#include <QCoreApplication>
#include <QDir>

#include <cerrno>
#include <sys/file.h>
#include <unistd.h>

namespace KDUpdater {

bool LockFile::Private::lock()
{
    if (locked)
        return true;

    errorString.clear();
    errno = 0;
    handle = open(filename.toLatin1().constData(), O_CREAT | O_RDWR | O_NONBLOCK, 0600);
    if (handle == -1) {
        errorString = QCoreApplication::translate("LockFile", "Cannot create lock file \"%1\": "
            "%2").arg(QDir::toNativeSeparators(filename), QString::fromLocal8Bit(strerror(errno)));
        return false;
    }
    const QString pid = QString::number(qApp->applicationPid());
    const QByteArray data = pid.toLatin1();
    errno = 0;
    qint64 written = 0;
    while (written < data.size()) {
        const qint64 n = write(handle, data.constData() + written, data.size() - written);
        if (n < 0) {
            errorString = QCoreApplication::translate("LockFile", "Cannot write PID to lock "
                "file \"%1\": %2").arg(QDir::toNativeSeparators(filename), QString::fromLocal8Bit(strerror(errno)));
            return false;
        }
        written += n;
    }
    errno = 0;
    locked = flock(handle, LOCK_NB | LOCK_EX) != -1;
    if (!locked) {
        errorString = QCoreApplication::translate("LockFile", "Cannot obtain the lock for \"%1\": %2.\n"
            "This likely means another instance of the installer is already running.\n"
            "Please close any other installer sessions and try again.")
            .arg(QDir::toNativeSeparators(filename), QString::fromLocal8Bit(strerror(errno)));
    }
    return locked;
}

bool LockFile::Private::unlock()
{
    errorString.clear();
    if (!locked)
        return true;

    errno = 0;
    locked = flock(handle, LOCK_UN | LOCK_NB) == -1;
    if (locked) {
        errorString = QCoreApplication::translate("LockFile", "Cannot release the lock for "
            "file \"%1\": %2").arg(QDir::toNativeSeparators(filename), QString::fromLocal8Bit(strerror(errno)));
    } else {
        unlink(filename.toLatin1());
    }
    return !locked;
}

} // namespace KDUpdater
