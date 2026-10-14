/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "keepaliveobject.h"
#include "remoteclient.h"

#include <QLocalSocket>
#include <QTimer>

namespace QInstaller {

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::KeepAliveObject
    \internal
*/

KeepAliveObject::KeepAliveObject()
    : m_timer(nullptr)
    , m_socket(nullptr)
{
}

void KeepAliveObject::start()
{
    if (m_timer)
        delete m_timer;
    m_timer = new QTimer(this);

    if (m_socket)
        delete m_socket;
    m_socket = new QLocalSocket(this);

    connect(m_timer, &QTimer::timeout, [this]() {
        if (m_socket->state() != QLocalSocket::UnconnectedState)
            return;
        m_socket->connectToServer(RemoteClient::instance().socketName());
    });

    connect(m_socket, &QLocalSocket::connected, [this]() {
        m_socket->close();
    });

    m_timer->setInterval(5000);
    m_timer->start();
}

} // namespace QInstaller
