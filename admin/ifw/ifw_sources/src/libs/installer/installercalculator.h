/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#ifndef INSTALLERCALCULATOR_H
#define INSTALLERCALCULATOR_H

#include "installer_global.h"
#include "qinstallerglobal.h"
#include "calculatorbase.h"

#include <QHash>
#include <QList>
#include <QSet>
#include <QString>

namespace QInstaller {

class Component;
class ComponentAlias;
class PackageManagerCore;

class INSTALLER_EXPORT InstallerCalculator : public CalculatorBase
{
public:
    InstallerCalculator(PackageManagerCore *core, const AutoDependencyHash &autoDependencyComponentHash);
    ~InstallerCalculator();

    bool solve();
    bool solve(const QList<Component *> &components) override;
    bool solve(const QList<ComponentAlias *> &aliases);

    QString resolutionText(const Component *component) const override;

private:
    bool solveComponent(Component *component, const QString &version = QString()) override;
    bool solveAlias(ComponentAlias *alias);

    void addComponentForInstall(Component *component, const QString &version = QString());
    bool addComponentsFromAlias(ComponentAlias *alias);
    QSet<Component *> autodependencyComponents();
    QString recursionError(Component *component) const;

    bool updateCheckState(Component *component, Qt::CheckState state);

private:
    QHash<Component*, QSet<Component*> > m_visitedComponents;
    QList<const Component*> m_componentsForAutodepencencyCheck;
    QSet<QString> m_toInstallComponentAliases;
    //Helper hash for quicker search for autodependency components
    AutoDependencyHash m_autoDependencyComponentHash;
};

} // namespace QInstaller

#endif // INSTALLERCALCULATOR_H
