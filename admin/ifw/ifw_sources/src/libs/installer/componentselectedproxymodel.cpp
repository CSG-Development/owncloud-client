/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "componentselectedproxymodel.h"
#include "componentmodel.h"
#include "fileutils.h"

namespace QInstaller {

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::ComponentSelectedProxyModel
    \internal
*/

ComponentSelectedProxyModel::ComponentSelectedProxyModel(PackageManagerCore *core, SelectionMode selectionMode, QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_core(core)
    , m_selectionMode(selectionMode)
{
    setRecursiveFilteringEnabled(true);
}

bool ComponentSelectedProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    ComponentModel *model = qobject_cast<ComponentModel*>(sourceModel());
    if (!model)
        return false;
    Component *component = model->componentFromIndex(sourceIndex);
    if (!component)
        return false;

    if (m_selectionMode == Unselected
            && (m_core->componentsToUninstallNames().contains(component->name()) || childSelected(component))) {
        return true;
    } else if (m_selectionMode == Selected
            && (m_core->componentsToInstallNames().contains(component->name()) || childSelected(component))) {
        return true;
    }

    return false;
}

QVariant ComponentSelectedProxyModel::data(const QModelIndex &index, int role) const
{
    // We are only interested in special handling of size column
    if (index.column() != ComponentModelHelper::UncompressedSizeColumn)
        return QSortFilterProxyModel::data(index, role);

    QModelIndex sourceIndex = mapToSource(index);
    ComponentModel *model = qobject_cast<ComponentModel*>(sourceModel());
    if (!model)
        return QVariant();
    Component *component = model->componentFromIndex(sourceIndex);
    if (!component)
        return QVariant();

    if (m_selectionMode == Unselected
            && (m_core->componentsToUninstallNames().contains(component->name()) || childSelected(component))) {
        return humanReadableSize(component->uncompressedSize(ComponentModelHelper::Uninstall));
    } else if (m_selectionMode == Selected
            && (m_core->componentsToInstallNames().contains(component->name()) || childSelected(component))) {
        return humanReadableSize(component->uncompressedSize(ComponentModelHelper::Install));
    }

    return QVariant();
}

bool ComponentSelectedProxyModel::childSelected(const Component *component) const
{
    if (!component->isInstalled())
        return false;
    const QList<Component*> descendants = component->descendantComponents();
    for (const Component *desc : descendants) {
        if (m_selectionMode == Selected && m_core->componentsToInstallNames().contains(desc->name()))
            return true;
        if (m_selectionMode == Unselected && m_core->componentsToUninstallNames().contains(desc->name()))
            return true;
    }
    return false;
}

} // namespace QInstaller
