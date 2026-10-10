/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <repository.h>
#include <settings.h>

#include <QDialog>
#include <QStyledItemDelegate>
#include <QTreeWidgetItem>

QT_BEGIN_NAMESPACE
class QAuthenticator;
class QLocale;
class QVariant;

namespace Ui {
    class SettingsDialog;
}
QT_END_NAMESPACE

namespace QInstaller {
    class PackageManagerCore;
}

// -- PasswordDelegate

class PasswordDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit PasswordDelegate(QWidget *parent = 0)
        : QStyledItemDelegate(parent)
        , m_showPasswords(true)
        , m_disabledEditor(true)
    {}

    void showPasswords(bool show);
    void disableEditing(bool disable);

protected:
    QString displayText(const QVariant &value, const QLocale &locale) const;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const;

private:
    bool m_showPasswords;
    bool m_disabledEditor;
};


// -- RepositoryItem

class RepositoryItem : public QTreeWidgetItem
{
public:
    explicit RepositoryItem(const QString &label);
    explicit RepositoryItem(const QInstaller::Repository &repo);

    QVariant data(int column, int role) const;
    void setData(int column, int role, const QVariant &value);

    QSet<QInstaller::Repository> repositories() const;
    QInstaller::Repository repository() const { return m_repo; }
    void setRepository(const QInstaller::Repository &repo) { m_repo = repo; }

private:
    QInstaller::Repository m_repo;
};


// -- SettingsDialog

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QInstaller::PackageManagerCore *core, QWidget *parent = 0);

    void showClearCacheProgress(bool show);

public slots:
    void accept();

signals:
    void networkSettingsChanged(const QInstaller::Settings &settings);
    void clearLocalCacheClicked();

private slots:
    void addRepository();
    void testRepository();
    void updatePasswords();
    void removeRepository();
    void useTmpRepositoriesOnly(bool use);
    void currentRepositoryChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void selectAll();
    void deselectAll();

private:
    void setupRepositoriesTreeWidget();
    void insertRepositories(const QSet<QInstaller::Repository> repos, QTreeWidgetItem *rootItem);
    void checkSubTree(QTreeWidgetItem *item, Qt::CheckState state);

private:
    Ui::SettingsDialog *m_ui;
    PasswordDelegate *m_delegate;
    QInstaller::PackageManagerCore *m_core;

    bool m_showPasswords;
    bool m_cacheCleared;
    QList<QTreeWidgetItem*> m_rootItems;
};

#endif  // SETTINGSDIALOG_H
