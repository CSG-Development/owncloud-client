/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#ifndef UNINSTALLERCALCULATOR_H
#define UNINSTALLERCALCULATOR_H

#include "installer_global.h"
#include "qinstallerglobal.h"
#include "calculatorbase.h"

#include <QHash>
#include <QList>
#include <QSet>
#include <QString>

namespace QInstaller {

class Component;
class PackageManagerCore;

class INSTALLER_EXPORT UninstallerCalculator : public CalculatorBase
{
public:
    UninstallerCalculator(PackageManagerCore *core,
                          const AutoDependencyHash &autoDependencyComponentHash,
                          const LocalDependencyHash &localDependencyComponentHash,
                          const QStringList &localVirtualComponents);
    ~UninstallerCalculator();

    bool solve(const QList<Component*> &components) override;
    QString resolutionText(const Component *component) const override;

private:
    bool solveComponent(Component *component, const QString &version = QString()) override;

    bool isRequiredVirtualPackage(Component *component);
    void appendVirtualComponentsToUninstall();

private:
    AutoDependencyHash m_autoDependencyComponentHash;
    LocalDependencyHash m_localDependencyComponentHash;
    QStringList m_localVirtualComponents;
};

} // namespace QInstaller

#endif // UNINSTALLERCALCULATOR_H
