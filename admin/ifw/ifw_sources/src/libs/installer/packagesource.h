/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef PACKAGESOURCE_H
#define PACKAGESOURCE_H

#include "installer_global.h"

#include <QUrl>

namespace QInstaller {

struct INSTALLER_EXPORT PackageSource
{
    PackageSource()
        : priority(-1)
    {}
    PackageSource(const QUrl &u, int p, bool pl = false)
        : url(u)
        , priority(p)
        , postLoadComponentScript(pl)
    {}

    QUrl url;
    int priority;
    bool postLoadComponentScript;
};

INSTALLER_EXPORT hashValue qHash(const PackageSource &key, hashValue seed);
INSTALLER_EXPORT bool operator==(const PackageSource &lhs, const PackageSource &rhs);

} // namespace QInstaller

#endif // PACKAGESOURCE_H
