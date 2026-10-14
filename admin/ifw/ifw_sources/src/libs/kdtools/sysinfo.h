/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef SYSINFO_H
#define SYSINFO_H

#include "kdtoolsglobal.h"

#include <QtCore/QString>

namespace KDUpdater {

class KDTOOLS_EXPORT VolumeInfo
{
public:
    VolumeInfo();
    static VolumeInfo fromPath(const QString &path);

    QString mountPath() const;
    void setMountPath(const QString &path);

    QString fileSystemType() const;
    void setFileSystemType(const QString &type);

    QString volumeDescriptor() const;
    void setVolumeDescriptor(const QString &descriptor);

    quint64 size() const;
    void setSize(const quint64 &size);

    quint64 availableSize() const;
    void setAvailableSize(const quint64 &available);

    bool operator==(const VolumeInfo &other) const;

private:
    QString m_mountPath;
    QString m_fileSystemType;
    QString m_volumeDescriptor;

    quint64 m_size;
    quint64 m_availableSize;
};

struct ProcessInfo
{
    quint32 id;
    QString name;
};

quint64 installedMemory();
QList<VolumeInfo> mountedVolumes();
QList<ProcessInfo> runningProcesses();
bool killProcess(const ProcessInfo &process, int msecs = 30000);
bool  pathIsOnLocalDevice(const QString &path);

} // namespace KDUpdater

QT_BEGIN_NAMESPACE
class QDebug;
QT_END_NAMESPACE

QDebug operator<<(QDebug dbg, KDUpdater::VolumeInfo volume);
QDebug operator<<(QDebug dbg, KDUpdater::ProcessInfo process);

#endif // SYSINFO_H
