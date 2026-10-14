/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "qinstallerglobal.h"

/*!
    \enum QInstaller::JobError

    \value InvalidUrl
    \value Timeout
    \value DownloadError
    \value InvalidUpdatesXml
    \value InvalidMetaInfo
    \value ExtractionError
    \value UserIgnoreError
    \value RepositoryUpdatesReceived
    \value CacheError
*/

/*!
    \typedef QInstaller::Operation

    Synonym for KDUpdater::UpdateOperation.
*/

/*!
    \typedef QInstaller::OperationList

    Synonym for QList<QInstaller::Operation*>.
*/

/*!
    \typedef QInstaller::Package

    Synonym for KDUpdater::Update.
*/

/*!
    \typedef QInstaller::PackagesList

    Synonym for QList<QInstaller::Package*>.
*/

/*!
    \typedef QInstaller::LocalPackagesMap

    Synonym for QMap<QString, KDUpdater::LocalPackage>. The map key is component name,
    and value contains information about the local package.
*/

/*!
    \typedef QInstaller::AutoDependencyHash

    Synonym for QHash<QString, QStringList>. The hash key is component name,
    that other components automatically depend on. The value can contain
    several component names, which are installed as an automatic dependency.
    For example:
    \badcode
    <Name>A</Name> //Hash value
    <AutoDependOn>B</AutoDependOn> //Hash key
    \endcode
*/

/*!
    \typedef QInstaller::LocalDependencyHash

    Synonym for QHash<QString, QStringList>. The hash key is component name,
    which other components depend on. The value can contain several component
    names, which depend on the key. For example:
    \badcode
    <Name>A</Name> //Hash value
    <Dependencies>B</Dependencies> //Hash key
    \endcode
*/
