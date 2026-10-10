/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "repositorymanager.h"
#include <QTemporaryFile>
#include <QUrl>
#include <QDate>
#include <QString>
#include <QMap>
#include <QMainWindow>
#include <QNetworkAccessManager>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


public slots:
    void displayRepositories();
    void getProductionRepository();
    void getUpdateRepository();
    void createExportFile();

private:
    void createRepositoryMap(const QByteArray &data, QMap<QString, ComponentDescription> &map);

    Ui::MainWindow *ui;
    RepositoryManager manager;
};

#endif // MAINWINDOW_H
