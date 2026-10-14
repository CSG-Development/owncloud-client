/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#ifndef REPOSITORYMANAGER_H
#define REPOSITORYMANAGER_H

#include <QObject>
#include <QMap>
#include <QDate>
#include <QNetworkAccessManager>

struct ComponentDescription {
    QString version;
    QDate releaseDate;
    QString checksum;
    QString updateText;
    bool update;
};

class RepositoryManager : public QObject
{
    Q_OBJECT
public:
    explicit RepositoryManager(QObject *parent = 0);

    bool updateRequired(const QString &componentName, QString *message = 0);
    QMap<QString, ComponentDescription>* productionComponents() { return &productionMap; }
    QMap<QString, ComponentDescription>* updateComponents() { return &updateMap; }
signals:
    void repositoriesCompared();

public slots:
    void setProductionRepository(const QString &repo);
    void setUpdateRepository(const QString &repo);
    void writeUpdateFile(const QString &fileName);

    void receiveRepository(QNetworkReply *reply);

    void compareRepositories();
private:
    void createRepositoryMap(const QByteArray &data, QMap<QString, ComponentDescription> &map);
    QNetworkReply *productionReply;
    QNetworkReply *updateReply;
    QNetworkAccessManager *manager;
    QMap<QString, ComponentDescription> productionMap;
    QMap<QString, ComponentDescription> updateMap;
};

#endif // REPOSITORYMANAGER_H
