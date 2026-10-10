/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef UPDATE_H
#define UPDATE_H

#include "packagesource.h"
#include "updatesinfo_p.h"
#include <QVariant>

namespace KDUpdater {

class Update
{
public:
    QVariant data(const QString &name, const QVariant &defaultValue = QVariant()) const;

    QInstaller::PackageSource packageSource() const {return m_packageSource; }

private:
    friend class UpdateFinder;
    Update(const QInstaller::PackageSource &packageSource, const UpdateInfo &updateInfo);

private:
    QInstaller::PackageSource m_packageSource;
    UpdateInfo m_updateInfo;
};

} // namespace KDUpdater

#endif // UPDATE_H
