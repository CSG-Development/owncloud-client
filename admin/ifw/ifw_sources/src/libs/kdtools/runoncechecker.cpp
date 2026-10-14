/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "runoncechecker.h"
#include "lockfile.h"
#include "sysinfo.h"
#include "globals.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QList>

#include <algorithm>

using namespace KDUpdater;

RunOnceChecker::RunOnceChecker(const QString &filename)
    : m_lockfile(filename)
{
}

RunOnceChecker::~RunOnceChecker()
{
    if (!m_lockfile.unlock())
        qCWarning(QInstaller::lcInstallerInstallLog).noquote() << m_lockfile.errorString();
}

class ProcessnameEquals
{
public:
    ProcessnameEquals(const QString &name)
#ifdef Q_OS_WIN
        : m_name(name.toLower())
#else
        : m_name(name)
#endif
    {}

    bool operator()(const ProcessInfo &info)
    {
#ifdef Q_OS_WIN
        const QString infoName = info.name.toLower();
        if (infoName == QDir::toNativeSeparators(m_name))
            return true;
#else
        const QString infoName = info.name;
#endif
        if (infoName == m_name)
            return true;

        const QFileInfo fi(infoName);
        if (fi.fileName() == m_name || fi.baseName() == m_name)
            return true;
        return false;
    }

private:
    QString m_name;
};

bool RunOnceChecker::isRunning(RunOnceChecker::ConditionFlags flags, const QString &appPath)
{
    if (flags.testFlag(ConditionFlag::ProcessList)) {
        const QList<ProcessInfo> allProcesses = runningProcesses();
        const int count = std::count_if(allProcesses.constBegin(), allProcesses.constEnd(),
            ProcessnameEquals(appPath));
        return (count > 1);
    }

    if (flags.testFlag(ConditionFlag::Lockfile)) {
        const bool locked = m_lockfile.lock();
        if (!locked)
            qCWarning(QInstaller::lcInstallerInstallLog).noquote() << m_lockfile.errorString();
        return !locked;
    }
    return false;
}
