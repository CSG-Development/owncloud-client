/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef UPDATEOPERATIONFACTORY_H
#define UPDATEOPERATIONFACTORY_H

#include "genericfactory.h"

#include "updater.h"

namespace QInstaller {
class PackageManagerCore;
}

namespace KDUpdater {

class UpdateOperation;

class KDTOOLS_EXPORT UpdateOperationFactory : public GenericFactory<UpdateOperation, QString,
                                                                   QInstaller::PackageManagerCore*>
{
    Q_DISABLE_COPY(UpdateOperationFactory)

public:
    static UpdateOperationFactory &instance();

    template <class T>
    void registerUpdateOperation(const QString &name)
    {
        registerProduct<T>(name);
    }

private:
    UpdateOperationFactory();
};

} // namespace KDUpdater

#endif // UPDATEOPERATIONFACTORY_H
