/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "packagesource.h"

namespace QInstaller {

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::PackageSource
    \brief The PackageSource class specifies a single package source.

    An package source represents a link to an repository that contains packages applicable by
    the installer or package maintenance application. This structure describes a single package
    source in terms of url and priority. While different repositories can host the same packages,
    packages coming from a higher priority source take precedence over lower priority packages
    during applicable package computation.
*/

/*!
    \fn QInstaller::PackageSource::PackageSource()

    Constructs an empty package source info object. The object's priority is set to -1. The url is
    initialized using a \l{default-constructed value}.
*/

/*!
    \fn QInstaller::PackageSource::PackageSource(const QUrl &u, int p, bool pl)

    Constructs a package source info object. The object's url is set to \a u, while the priority
    is set to \a p.
*/

/*!
    \variable QInstaller::PackageSource::url
    \brief The URL of the package source.
*/

/*!
    \variable QInstaller::PackageSource::priority
    \brief The priority of the package source.
*/

/*!
    Returns the hash value for the \a key, using \a seed to seed the calculation.
*/
hashValue qHash(const PackageSource &key, hashValue seed)
{
    return qHash(key.url, seed) ^ key.priority;
}

/*!
    Returns \c true if \a lhs and \a rhs are equal; otherwise returns \c false.
*/
bool operator==(const PackageSource &lhs, const PackageSource &rhs)
{
    return lhs.url == rhs.url && lhs.priority == rhs.priority;
}

} // namespace QInstaller
