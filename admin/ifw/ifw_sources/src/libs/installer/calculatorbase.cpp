/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "calculatorbase.h"

#include "component.h"

namespace QInstaller {

CalculatorBase::CalculatorBase(PackageManagerCore *core)
    : m_core(core)
{
}

CalculatorBase::~CalculatorBase()
{
}

void CalculatorBase::insertResolution(Component *component, const Resolution resolutionType
    , const QString &referencedComponent)
{
    // Keep the first reason
    if (m_componentNameResolutionHash.contains(component->name()))
        return;

    m_componentNameResolutionHash.insert(component->name(),
        QPair<Resolution, QString>(resolutionType, referencedComponent));
}

QList<Component *> CalculatorBase::resolvedComponents() const
{
    return m_resolvedComponents;
}

QSet<QString> CalculatorBase::resolvedComponentNames() const
{
    return m_resolvedComponentNames;
}

CalculatorBase::Resolution CalculatorBase::resolutionType(const Component *component) const
{
    return m_componentNameResolutionHash.value(component->name()).first;
}

QString CalculatorBase::error() const
{
    return m_errorString;
}

QString CalculatorBase::referencedComponent(const Component *component) const
{
    return m_componentNameResolutionHash.value(component->name()).second;
}

} // namespace QInstaller

