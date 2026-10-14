/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef REMOTESERVERCONNECTION_H
#define REMOTESERVERCONNECTION_H

#include "abstractarchive.h"

#include <QPointer>
#include <QThread>
#include <QProcess>

#include <QtCore/private/qfsfileengine_p.h>

QT_BEGIN_NAMESPACE
class QLocalSocket;
QT_END_NAMESPACE

namespace QInstaller {

class PermissionSettings;

class QProcessSignalReceiver;
class AbstractArchiveSignalReceiver;

class RemoteServerReply
{
public:
    explicit RemoteServerReply(QLocalSocket *socket);
    ~RemoteServerReply();

    template <typename T>
    void send(const T &data);

private:
    QLocalSocket *m_socket;
    bool m_sent;
};

class RemoteServerConnection : public QThread
{
    Q_OBJECT
    Q_DISABLE_COPY(RemoteServerConnection)

public:
    RemoteServerConnection(qintptr socketDescriptor, const QString &authorizationKey,
                           QObject *parent);

    void run() override;

signals:
    void shutdownRequested();

private:
    void handleQProcess(RemoteServerReply *reply, const QString &command, QDataStream &data);
    void handleQSettings(RemoteServerReply *reply, const QString &command, QDataStream &data,
                         PermissionSettings *settings);
    void handleQFSFileEngine(RemoteServerReply *reply, const QString &command, QDataStream &data);
    void handleArchive(RemoteServerReply *reply, const QString &command, QDataStream &data);

private:
    qintptr m_socketDescriptor;
    QString m_authorizationKey;

    QScopedPointer<QProcess> m_process;
    QScopedPointer<QFSFileEngine> m_engine;
    QScopedPointer<AbstractArchive> m_archive;

    QProcessSignalReceiver *m_processSignalReceiver;
    AbstractArchiveSignalReceiver *m_archiveSignalReceiver;
};

} // namespace QInstaller

#endif // REMOTESERVERCONNECTION_H
