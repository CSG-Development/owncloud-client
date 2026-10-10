/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "update.h"

using namespace KDUpdater;

/*!
   \inmodule kdupdater
   \class KDUpdater::Update
   \brief Represents a single update.

   The KDUpdater::Update class contains information about an update. It is created by KDUpdater::UpdateFinder
   corresponding to the update.

   The constructor of the KDUpdater::Update class is made protected, because it can be instantiated only by
   KDUpdater::UpdateFinder (which is a friend class). The destructor however is public.
*/

/*!
    \fn KDUpdater::Update::packageSource() const

    Returns the package source.
*/

/*!
   \internal
*/
Update::Update(const QInstaller::PackageSource &packageSource, const UpdateInfo &updateInfo)
    : m_packageSource(packageSource)
    , m_updateInfo(updateInfo)
{
}

/*!
   Returns the data specified by \a name, or an invalid \a defaultValue if the
   data does not exist.
*/
QVariant Update::data(const QString &name, const QVariant &defaultValue) const
{
    return m_updateInfo.data.value(name, defaultValue);
}
