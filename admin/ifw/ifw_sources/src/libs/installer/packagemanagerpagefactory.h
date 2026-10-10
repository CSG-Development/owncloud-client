/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef PACKAGEMANAGERPAGEFACTORY_H
#define PACKAGEMANAGERPAGEFACTORY_H

#include "genericfactory.h"
#include "qinstallerglobal.h"

namespace QInstaller {

class PackageManagerCore;
class PackageManagerPage;

class INSTALLER_EXPORT PackageManagerPageFactory : public GenericFactory<PackageManagerPage, int,
                                                                           PackageManagerCore*>
{
    Q_DISABLE_COPY(PackageManagerPageFactory)

public:
    static PackageManagerPageFactory &instance();
    template<typename T> void registerPackageManagerPage(int id)
    {
        registerProduct<T>(id);
    }

private:
    PackageManagerPageFactory() = default;
};

} // namespace QInstaller

#endif // PACKAGEMANAGERPAGEFACTORY_H
