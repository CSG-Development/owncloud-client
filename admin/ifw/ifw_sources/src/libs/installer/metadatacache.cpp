/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "metadatacache.h"

#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)

namespace QInstaller {

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::MetadataCache
    \brief The MetadataCache is a class for a checksum based storage of \c Metadata objects on disk.

    MetadataCache manages a cache storage for a set \l{path()}, which contains
    a subdirectory for each registered \c Metadata item. The cache has a manifest file in
    its root directory, which lists the version and type of the cache, and all its items.
    The file is updated automatically when the metadata cache object is destructed, or
    it can be updated periodically by calling \l{sync()}.
*/

/*!
    Constructs a new empty cache. The cache is invalid until set with a
    path and initialized.
*/
MetadataCache::MetadataCache()
    : GenericDataCache<Metadata>()
{
    setType(QLatin1String("Metadata"));
    setVersion(QLatin1String(QUOTE(IFW_CACHE_FORMAT_VERSION)));
}

/*!
    Constructs a cache to \a path. The cache is initialized automatically.
*/
MetadataCache::MetadataCache(const QString &path)
    : GenericDataCache(path, QLatin1String("Metadata"), QLatin1String(QUOTE(IFW_CACHE_FORMAT_VERSION)))
{
}

} // namespace QInstaller
