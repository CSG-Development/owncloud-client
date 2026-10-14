/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef KEEPALIVEOBJECT_H
#define KEEPALIVEOBJECT_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QTimer;
class QLocalSocket;
QT_END_NAMESPACE

namespace QInstaller {

class KeepAliveObject : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(KeepAliveObject)

public:
    KeepAliveObject();

public slots:
    void start();

private:
    QTimer *m_timer;
    QLocalSocket *m_socket;
};

} // namespace QInstaller

#endif // KEEPALIVEOBJECT_H
