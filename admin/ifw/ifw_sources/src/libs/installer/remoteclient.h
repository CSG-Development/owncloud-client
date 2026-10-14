/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef REMOTECLIENT_H
#define REMOTECLIENT_H

#include "installer_global.h"
#include "protocol.h"

#include <QScopedPointer>

namespace QInstaller {

class RemoteClientPrivate;

class INSTALLER_EXPORT RemoteClient
{
    Q_DISABLE_COPY(RemoteClient)
    Q_DECLARE_PRIVATE(RemoteClient)

public:
    static RemoteClient &instance();
    void init(const QString &socketName, const QString &key, Protocol::Mode mode,
              Protocol::StartAs startAs);
    void setAuthorizationFallbackDisabled(bool disabled);

    void shutdown();
    void destroy();

    QString socketName() const;
    QString authorizationKey() const;
    QString socketPathName(const QString &socketName) const;

    bool isActive() const;
    void setActive(bool active);

private:
    RemoteClient();
    ~RemoteClient();

private:
    static RemoteClient *s_instance;
    QScopedPointer<RemoteClientPrivate> d_ptr;
};

} // namespace QInstaller

#endif // REMOTECLIENT_H
