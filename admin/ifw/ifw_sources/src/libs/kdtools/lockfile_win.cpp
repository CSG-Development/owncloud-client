/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "lockfile.h"
#include "lockfile_p.h"

#include "utils.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

namespace KDUpdater {

bool LockFile::Private::lock()
{
    if (locked)
        return locked;

    errorString.clear();
    handle = CreateFile(filename.toStdWString().data(), GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ, NULL, QFileInfo::exists(filename) ? OPEN_EXISTING : CREATE_NEW,
        FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
        errorString = QCoreApplication::translate("LockFile", "Cannot create lock file \"%1\": "
            "%2").arg(QDir::toNativeSeparators(filename), QInstaller::windowsErrorString(GetLastError()));
        return false;
    }

    DWORD bytesWritten;
    const QByteArray pid = QString::number(QCoreApplication::applicationPid()).toLatin1();
    if (!WriteFile(handle, pid.data(), pid.size(), &bytesWritten, NULL)) {
        errorString = QCoreApplication::translate("LockFile", "Cannot write PID to lock file "
            "\"%1\": %2").arg(QDir::toNativeSeparators(filename), QInstaller::windowsErrorString(GetLastError()));
        return false;
    }
    FlushFileBuffers(handle);

    if (!::LockFile(handle, 0, 0, QFileInfo(filename).size(), 0)) {
        errorString = QCoreApplication::translate("LockFile",
            "Cannot obtain the lock for \"%1\": %2.\n"
            "This likely means another instance of the installer is already running.\n"
            "Please close any other installer sessions and try again.")
            .arg(QDir::toNativeSeparators(filename), QInstaller::windowsErrorString(GetLastError()));
    } else {
        locked = true;
    }
    return locked;
}

bool LockFile::Private::unlock()
{
    errorString.clear();
    if (!locked)
        return true;

    if (!UnlockFile(handle, 0, 0, QFileInfo(filename).size(), 0)) {
        errorString = QCoreApplication::translate("LockFile", "Cannot release the lock for "
            "file \"%1\": %2").arg(QDir::toNativeSeparators(filename), QInstaller::windowsErrorString(GetLastError()));
    } else {
        locked = false;
        CloseHandle(handle);
    }
    return !locked;
}

} // namespace KDUpdater
