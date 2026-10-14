/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef QINSTALLER_GLOBAL_H
#define QINSTALLER_GLOBAL_H

#include <installer_global.h>

#include "update.h"
#include "updateoperation.h"
#include "localpackagehub.h"

namespace QInstaller {

enum INSTALLER_EXPORT JobError
{
    InvalidUrl = 0x24B04,
    Timeout,
    DownloadError,
    MetaDownloadError,
    InvalidUpdatesXml,
    InvalidMetaInfo,
    ExtractionError,
    UserIgnoreError,
    RepositoryUpdatesReceived,
    CacheError
};

typedef KDUpdater::UpdateOperation Operation;
typedef QList<QInstaller::Operation*> OperationList;

typedef KDUpdater::Update Package;
typedef QList<QInstaller::Package*> PackagesList;

typedef QMap<QString, KDUpdater::LocalPackage> LocalPackagesMap;

typedef QHash<QString, QStringList> AutoDependencyHash;
typedef QHash<QString, QStringList> LocalDependencyHash;

} // namespace QInstaller

#endif // QINSTALLER_GLOBAL_H
