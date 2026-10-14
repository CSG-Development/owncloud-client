/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef CALCULATORBASE_H
#define CALCULATORBASE_H

#include "installer_global.h"

#include <QList>
#include <QString>
#include <QMetaEnum>

namespace QInstaller {

class Component;
class PackageManagerCore;

class INSTALLER_EXPORT CalculatorBase
{
public:
    enum class Resolution {
        Selected = 0,     // "Selected Component(s) without Dependencies" / "Deselected Component(s)"
        Replaced,         // "Component(s) replaced by other components"
        VirtualDependent, // "No dependencies to virtual component"
        Dependent,        // "Added as dependency for %1." / "Removed as dependency component is removed"
        Automatic,        // "Component(s) added as automatic dependencies" / "Removed as autodependency component is removed"
        Resolved,         // "Component(s) that have resolved Dependencies"
        Alias             // "Components added from selected alias"
    };

    CalculatorBase(PackageManagerCore *core);
    virtual ~CalculatorBase() = 0;

    virtual bool solve(const QList<Component *> &components) = 0;
    void insertResolution(Component *component, const Resolution resolutionType,
                          const QString &referencedComponent = QString());

    QList<Component *> resolvedComponents() const;
    QSet<QString> resolvedComponentNames() const;
    virtual QString resolutionText(const Component *component) const = 0;
    Resolution resolutionType(const Component *component) const;

    QString error() const;

protected:
    virtual bool solveComponent(Component *component, const QString &version = QString()) = 0;
    QString referencedComponent(const Component *component) const;

protected:
    PackageManagerCore *m_core;
    QString m_errorString;

    QList<Component *> m_resolvedComponents;
    QSet<QString> m_resolvedComponentNames;  //for faster lookups
    QHash<QString, QPair<Resolution, QString> > m_componentNameResolutionHash;
};

} // namespace QInstaller

Q_DECLARE_METATYPE(QInstaller::CalculatorBase::Resolution)

#endif // CALCULATORBASE_H
