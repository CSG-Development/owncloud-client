/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include <packagemanagerpagefactory.h>

namespace QInstaller {

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::PackageManagerPageFactory
    \internal
*/

PackageManagerPageFactory &PackageManagerPageFactory::instance()
{
    static PackageManagerPageFactory factory;
    return factory;
}

}
