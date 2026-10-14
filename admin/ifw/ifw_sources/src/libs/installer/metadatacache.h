/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef METADATACACHE_H
#define METADATACACHE_H

#include "genericdatacache.h"
#include "metadata.h"

namespace QInstaller {

class MetadataCache : public GenericDataCache<Metadata>
{
public:
    MetadataCache();
    explicit MetadataCache(const QString &path);
};

} // namespace QInstaller

#endif // METADATACACHE_H
