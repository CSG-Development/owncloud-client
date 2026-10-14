/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "sysinfo.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>

using namespace KDUpdater;

struct PathLongerThan
{
    bool operator()(const VolumeInfo &lhs, const VolumeInfo &rhs) const
    {
        return lhs.mountPath().length() > rhs.mountPath().length();
    }
};

VolumeInfo::VolumeInfo()
    : m_size(0)
    , m_availableSize(0)
{
}

VolumeInfo VolumeInfo::fromPath(const QString &path)
{
    QDir targetPath(QDir::cleanPath(path));
    QList<VolumeInfo> volumes = mountedVolumes();

    // sort by length to get the longest mount point (not just "/") first
    std::sort(volumes.begin(), volumes.end(), PathLongerThan());
    foreach (const VolumeInfo &volume, volumes) {
        const QDir volumePath(volume.mountPath());
        if (targetPath == volumePath)
            return volume;
#ifdef Q_OS_WIN
        if (QDir::toNativeSeparators(path).toLower().startsWith(volume.mountPath().toLower()))
#else
        // we need to take some care here, as canonical path might return an empty string if the target
        // does not exist yet
        if (targetPath.exists()) {
            // the target exist, we can solve the path and if it fits return
            if (targetPath.canonicalPath().startsWith(volume.mountPath()))
                return volume;
            continue;
        }

        // the target directory does not exist yet, we need to cd up till we find the first existing dir
        QStringList parts = targetPath.absolutePath().split(QDir::separator(), Qt::SkipEmptyParts);
        while (targetPath.absolutePath() != QDir::rootPath()) {
            if (targetPath.exists())
                break;
            parts.pop_back();
            if (parts.isEmpty())
                targetPath = QDir(QDir::rootPath());
            else
                targetPath = QDir(QLatin1Char('/') + parts.join(QDir::separator()));
        }

        if (targetPath.canonicalPath().startsWith(volume.mountPath()))
#endif
            return volume;
    }
    return VolumeInfo();
}

QString VolumeInfo::mountPath() const
{
    return m_mountPath;
}

void VolumeInfo::setMountPath(const QString &path)
{
    m_mountPath = path;
}

QString VolumeInfo::fileSystemType() const
{
    return m_fileSystemType;
}

void VolumeInfo::setFileSystemType(const QString &type)
{
    m_fileSystemType = type;
}

QString VolumeInfo::volumeDescriptor() const
{
    return m_volumeDescriptor;
}

void VolumeInfo::setVolumeDescriptor(const QString &descriptor)
{
    m_volumeDescriptor = descriptor;
}

quint64 VolumeInfo::size() const
{
    return m_size;
}

void VolumeInfo::setSize(const quint64 &size)
{
    m_size = size;
}

quint64 VolumeInfo::availableSize() const
{
    return m_availableSize;
}

void VolumeInfo::setAvailableSize(const quint64 &available)
{
    m_availableSize = available;
}

bool VolumeInfo::operator==(const VolumeInfo &other) const
{
    return m_volumeDescriptor == other.m_volumeDescriptor;
}

QDebug operator<<(QDebug dbg, const VolumeInfo &volume)
{
    return dbg << "KDUpdater::Volume(" << volume.mountPath() << ")";
}

QDebug operator<<(QDebug dbg, ProcessInfo process)
{
    return dbg << "KDUpdater::ProcessInfo(" << process.id << ", " << process.name << ")";
}
