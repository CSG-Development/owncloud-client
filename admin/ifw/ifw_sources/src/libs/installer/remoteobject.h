/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef REMOTEOBJECT_H
#define REMOTEOBJECT_H

#include "errors.h"
#include "installer_global.h"
#include "protocol.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QLocalSocket>
#include <QObject>
#include <QVariant>


namespace QInstaller {

class INSTALLER_EXPORT RemoteObject : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(RemoteObject)

public:
    RemoteObject(const QString &wrappedType, QObject *parent = 0);
    virtual ~RemoteObject() = 0;

    bool isConnectedToServer() const;

    template<typename... Args>
    void callRemoteMethodDefaultReply(const QString &name, const Args&... args)
    {
        const QString reply = sendReceivePacket<QString>(name, args...);
        Q_ASSERT(reply == QLatin1String(Protocol::DefaultReply));
    }

    template<typename T, typename... Args>
    T callRemoteMethod(const QString &name, const Args&... args) const
    {
        return sendReceivePacket<T>(name, args...);
    }

protected:
    bool authorize();
    bool connectToServer(const QVariantList &arguments = QVariantList());

private:

    template<typename T, typename... Args>
    T sendReceivePacket(const QString &name, const Args&... args) const
    {
        writeData(name, args...);
        while (m_socket->bytesToWrite())
            m_socket->waitForBytesWritten();

        return readData<T>(name);
    }

    template <class T> int writeObject(QDataStream& out, const T& t) const
    {
        static_assert(!std::is_pointer<T>::value, "Pointer passed to remote server");
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        out << t;
#else
        if constexpr (std::is_same<T, QAnyStringView>::value)
            out << t.toString();
        else
            out << t;
#endif

        return 0;
    }

    template<typename... Args>
    void writeData(const QString &name, const Args&... args) const
    {
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);

        (void)std::initializer_list<int>{writeObject(out, args)...};
        sendPacket(m_socket, name.toLatin1(), data);
        m_socket->flush();
    }

    template<typename T>
    T readData(const QString &name) const
    {
        QByteArray command;
        QByteArray data;
        while (!receivePacket(m_socket, &command, &data)) {
            if (!m_socket->waitForReadyRead(-1)) {
                throw Error(tr("Cannot read all data after sending command: %1. "
                    "Bytes expected: %2, Bytes received: %3. Error: %4").arg(name).arg(0)
                    .arg(m_socket->bytesAvailable()).arg(m_socket->errorString()));
            }
        }

        Q_ASSERT(command == Protocol::Reply);

        QDataStream stream(&data, QIODevice::ReadOnly);

        T result;
        stream >> result;
        Q_ASSERT(stream.status() == QDataStream::Ok);
        Q_ASSERT(stream.atEnd());
        return result;
    }

private:
    QString m_type;
    QLocalSocket *m_socket;
};

} // namespace QInstaller

#endif // REMOTEOBJECT_H
