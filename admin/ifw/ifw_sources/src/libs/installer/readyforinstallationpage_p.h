/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef READYFORINSTALLATIONPAGE_P_H
#define READYFORINSTALLATIONPAGE_P_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QTreeView>
#include <QApplication>

#include "packagemanagergui.h"
#include "packagemanagercore.h"
#include "componentselectedproxymodel.h"

namespace Ui {
    class ComponentSummaryForm;
}

namespace QInstaller {

class SpaceWidget;

class TreeItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TreeItemDelegate(QObject *parent);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class ReadyForInstallationPagePrivate : public QObject
{
    Q_OBJECT
    friend class ReadyForInstallationPage;
    Q_DISABLE_COPY(ReadyForInstallationPagePrivate)

public:
    ReadyForInstallationPagePrivate(ReadyForInstallationPage *qq, PackageManagerCore *core);
    ~ReadyForInstallationPagePrivate();

private:
    bool entering();
    void initializeTitleLabel(QLabel *label);
    void initializeInstallTree(const QString &title);
    void initializeUninstallTree();
    void initializeTree(QSortFilterProxyModel *model, QTreeView *treeView, QWidget *container);
    void initializeUninstaller();
    void updateView(QSortFilterProxyModel *model, QTreeView *treeView, QWidget *container);
    void decorateTreeView(QTreeView *treeView);
    void adjustTreeWidgetHeight(QTreeView *treeView);
    void showSpaceWidget(bool show);

private:
    Ui::ComponentSummaryForm *m_ui;
    ReadyForInstallationPage *q;
    PackageManagerCore *m_core;
    SpaceWidget *m_spaceWidget;
    ComponentSelectedProxyModel *m_componentsSelectedProxyModel;
    ComponentSelectedProxyModel *m_componentsUnselectedProxyModel;
};

} // namespace QInstaller

#endif // READYFORINSTALLATIONPAGE_P_H
