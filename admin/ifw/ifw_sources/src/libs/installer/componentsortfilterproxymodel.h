/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef COMPONENTSORTFILTERPROXYMODEL_H
#define COMPONENTSORTFILTERPROXYMODEL_H

#include "installer_global.h"

#include <QSortFilterProxyModel>

namespace QInstaller {

class INSTALLER_EXPORT ComponentSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    enum AcceptType {
        Direct,
        Descendant,
        Rejected
    };

    explicit ComponentSortFilterProxyModel(QObject *parent = nullptr);

    QVector<QModelIndex> directlyAcceptedIndexes() const;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    bool acceptsRow(int sourceRow, const QModelIndex &sourceParent, AcceptType *type = nullptr) const;
    bool findDirectlyAcceptedIndexes(const QModelIndex &in, QVector<QModelIndex> &indexes) const;
};

} // namespace QInstaller

#endif // COMPONENTSORTFILTERPROXYMODEL_H
