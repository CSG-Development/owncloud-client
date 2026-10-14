/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#include <QApplication>
#include <QTimer>
#include <QtCore>
#include "mainwindow.h"
#include "repositorymanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName(QLatin1String("IFW_repocompare"));
    if (a.arguments().contains(QLatin1String("-i"))) {
        if (a.arguments().count() != 5) {
            qWarning() << "Usage: " << a.arguments().at(0) << " -i <production Repo> <update Repo> <outputFile>";
            return -1;
        }
        const QString productionRepo = a.arguments().at(2);
        const QString updateRepo = a.arguments().at(3);
        const QString outputFile = a.arguments().at(4);
        RepositoryManager manager;
        manager.setProductionRepository(productionRepo);
        manager.setUpdateRepository(updateRepo);
        a.connect(&manager, &RepositoryManager::repositoriesCompared, &a, &QApplication::quit);
        qDebug() << "Waiting for server reply...";
        a.exec();
        qDebug() << "Writing into " << outputFile;
        manager.writeUpdateFile(outputFile);
        return 0;
    } else {
        MainWindow w;
        w.show();

        return a.exec();
    }
}
