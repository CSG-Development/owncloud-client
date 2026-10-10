/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef COMPONENTSELECTEDPROXYMODEL_H
#define COMPONENTSELECTEDPROXYMODEL_H

#include "installer_global.h"
#include "packagemanagercore.h"

#include <QSortFilterProxyModel>

namespace QInstaller {

enum SelectionMode {
    Selected,
    Unselected
};

class INSTALLER_EXPORT ComponentSelectedProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit ComponentSelectedProxyModel(PackageManagerCore *core, SelectionMode selectionMode, QObject *parent = nullptr);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    bool childSelected(const Component *component) const;

private:
    PackageManagerCore *m_core;
    SelectionMode m_selectionMode;
};

} // namespace QInstaller

#endif // COMPONENTSELECTEDPROXYMODEL_H
