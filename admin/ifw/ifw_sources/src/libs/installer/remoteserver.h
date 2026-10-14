/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef REMOTESERVER_H
#define REMOTESERVER_H

#include "installer_global.h"
#include "protocol.h"

#include <QObject>

namespace QInstaller {

class RemoteServerPrivate;

class INSTALLER_EXPORT RemoteServer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(RemoteServer)
    Q_DECLARE_PRIVATE(RemoteServer)

public:
    explicit RemoteServer(QObject *parent = 0);
    ~RemoteServer();

    void start();
    void init(const QString &socketName, const QString &authorizationKey, Protocol::Mode mode);

    QString socketName() const;
    QString authorizationKey() const;
    QString socketPathName(const QString &socketName) const;

private slots:
    void restartWatchdog();

private:
    QScopedPointer<RemoteServerPrivate> d_ptr;
};

} // namespace QInstaller

#endif // REMOTESERVER_H
