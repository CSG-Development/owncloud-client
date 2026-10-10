/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "directoryguard.h"

#include "fileutils.h"
#include "globals.h"
#include "errors.h"

#include <QCoreApplication>
#include <QDir>

namespace QInstaller {

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::DirectoryGuard
    \brief RAII class to create a directory and delete it on destruction unless released.
*/

/*!
    Constructs a new guard object for \a path.
*/
DirectoryGuard::DirectoryGuard(const QString &path)
    : m_path(path)
    , m_created(false)
    , m_released(false)
{
    m_path.replace(QLatin1Char('\\'), QLatin1Char('/'));
}

/*!
    Destroys the directory guard instance and removes the
    guarded directory unless released.
*/
DirectoryGuard::~DirectoryGuard()
{
    if (!m_created || m_released)
        return;
    QDir dir(m_path);
    if (!dir.rmdir(m_path))
        qCWarning(lcInstallerInstallLog) << "Cannot delete directory" << m_path;
}

/*!
    Tries to create the directory structure.
    Returns a list of every directory created.
*/
QStringList DirectoryGuard::tryCreate()
{
    if (m_path.isEmpty())
        return QStringList();

    const QFileInfo fi(m_path);
    if (fi.exists() && fi.isDir())
        return QStringList();
    if (fi.exists() && !fi.isDir()) {
        throw Error(QCoreApplication::translate("DirectoryGuard",
            "Path \"%1\" exists but is not a directory.").arg(QDir::toNativeSeparators(m_path)));
    }
    QStringList created;

    QDir toCreate(m_path);
    while (!toCreate.exists()) {
        QString p = toCreate.absolutePath();
        created.push_front(p);
        p = p.section(QLatin1Char('/'), 0, -2);
        toCreate = QDir(p);
    }

    m_created = QInstaller::createDirectoryWithParents(m_path);
    if (!m_created) {
        throw Error(QCoreApplication::translate("DirectoryGuard",
            "Cannot create directory \"%1\".").arg(QDir::toNativeSeparators(m_path)));
    }
    return created;
}

/*!
    Marks the directory as released.
*/
void DirectoryGuard::release()
{
    m_released = true;
}

} // namespace QInstaller
