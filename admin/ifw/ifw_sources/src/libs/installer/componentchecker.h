/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#ifndef COMPONENTCHECKER_H
#define COMPONENTCHECKER_H

#include "installer_global.h"
#include <QCoreApplication>

namespace QInstaller {

class Component;

class INSTALLER_EXPORT ComponentChecker
{
    Q_DECLARE_TR_FUNCTIONS(ComponentChecker)
public:
    static QStringList checkComponent(Component *component);

private:
    ComponentChecker();
};

}


#endif // INSTALLERCALCULATOR_H
