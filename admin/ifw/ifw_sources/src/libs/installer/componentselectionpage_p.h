/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef COMPONENTSELECTIONPAGE_P_H
#define COMPONENTSELECTIONPAGE_P_H

#include <QObject>
#include <QWidget>
#include <QHeaderView>
#include <QComboBox>

#include "componentmodel.h"
#include "packagemanagergui.h"
#include "componentsortfilterproxymodel.h"

class QTreeView;
class QLabel;
class QScrollArea;
class QPushButton;
class QGroupBox;
class QListWidgetItem;
class QProgressBar;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QStackedLayout;

namespace QInstaller {

class PackageManagerCore;
class ComponentModel;
class ComponentSelectionPage;
class CategoryComboBox;
class ClickableLabel;
class SpaceWidget;

class ComponentSelectionPagePrivate : public QObject
{
    Q_OBJECT
    friend class ComponentSelectionPage;
    Q_DISABLE_COPY(ComponentSelectionPagePrivate)

public:
    explicit ComponentSelectionPagePrivate(ComponentSelectionPage *qq, PackageManagerCore *core);
    ~ComponentSelectionPagePrivate();

    void setAllowCreateOfflineInstaller(bool allow);
    void showCompressedRepositoryButton();
    void hideCompressedRepositoryButton();
    void showCreateOfflineInstallerButton(bool show);
    void showRepositoryCategories();
    void setAdvancedTitleVisibility();
    void updateTreeView();
    void expandDefault();
    void expandSearchResults();
    bool componentsResolved() const;
    QString targetDir() const;

public slots:
    void currentSelectedChanged(const QModelIndex &current);
    void selectAll();
    void deselectAll();
    void updateWidgetVisibility(bool show);
    void fetchRepositoryCategories();
    void createOfflineButtonClicked();
    void qbspButtonClicked();
    void onProgressChanged(int progress);
    void setMessage(const QString &msg);
    void setTotalProgress(int totalProgress);
    void selectDefault();
    void onModelStateChanged(QInstaller::ComponentModel::ModelState state);
    void setSearchPattern(const QString &text);
    void targetDirBrowse();

private:
    void storeHeaderResizeModes();
    void restoreHeaderResizeModes();
    void addCategorySelection(QHBoxLayout *topHLayout);

private:
    ComponentSelectionPage *q;
    PackageManagerCore *m_core;
    QTreeView *m_treeView;
    QWidget *m_descriptionBaseWidget;
    QLabel *m_sizeLabel;
    QLabel *m_descriptionLabel;
    QPushButton *m_createOfflinePushButton;
    QPushButton *m_qbspPushButton;
    QWidget *m_categoryWidget;
    QGroupBox *m_categoryGroupBox;
    QLabel *m_metadataProgressLabel;
    QProgressBar *m_progressBar;
    QGridLayout *m_mainGLayout;
    QHBoxLayout *m_topHLayout;
    QVBoxLayout *m_rightSideVLayout;
    bool m_allowCreateOfflineInstaller;
    bool m_categoryLayoutVisible;
    ComponentModel *m_allModel;
    ComponentModel *m_updaterModel;
    ComponentModel *m_currentModel;
    QStackedLayout *m_stackedLayout;
    ComponentSortFilterProxyModel *m_proxyModel;
    QLineEdit *m_searchLineEdit;
    bool m_componentsResolved;
    CategoryComboBox* m_categoryCombobox;
    ClickableLabel *m_selectAll;
    ClickableLabel *m_selectNone;
    ClickableLabel *m_reset;
    QLabel *m_advancedTitle;
    SpaceWidget *m_spaceWidget;
    QAction *m_searchAction;
    QLabel *m_targetDirLabel;
    QPushButton *m_targetDirBrowseButton;
    QLabel *m_targetDirWarningLabel;

    bool m_headerStretchLastSection;
    QHash<int, QHeaderView::ResizeMode> m_headerResizeModes;

signals:
    void targetDirChanged();
};

}  // namespace QInstaller

#endif // COMPONENTSELECTIONPAGE_P_H
