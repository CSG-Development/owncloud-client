/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "protocol.h"
#include <QIODevice>

namespace QInstaller {

typedef qint32 PackageSize;

/*!
    \inmodule QtInstallerFramework
    \namespace QInstaller::Protocol
    \brief Contains values related to the internal client-server connection protocol.
*/

/*!
    \enum QInstaller::Protocol::Mode

    \value Debug
    \value Production
*/

/*!
    \enum QInstaller::Protocol::StartAs

    \value User
    \value SuperUser
*/

/*!
    Write a packet containing \a command and \a data to \a device.

    \note Both client and server need to have the same endianness.
 */
void sendPacket(QIODevice *device, const QByteArray &command, const QByteArray &data)
{
    // use aliasing for writing payload size into bytes
    char payloadBytes[sizeof(PackageSize)];
    PackageSize *payloadSize = reinterpret_cast<PackageSize*>(&payloadBytes);
    *payloadSize = command.size() + sizeof(char) + data.size();

    QByteArray packet;
    packet.reserve(sizeof(PackageSize) + *payloadSize);
    packet.append(payloadBytes, sizeof(PackageSize));
    packet.append(command);
    packet.append('\0');
    packet.append(data);

    forever {
        const int bytesWritten = device->write(packet);
        Q_ASSERT(bytesWritten >= 0);
        if (bytesWritten == packet.size())
            break;
        packet.remove(0, bytesWritten);
    }
}

/*!
    Reads a packet from \a device, and stores its content into \a command and \a data.

    Returns \c false if the packet in the device buffer is yet incomplete, \c true otherwise.

    \note Both client and server need to have the same endianness.
 */
bool receivePacket(QIODevice *device, QByteArray *command, QByteArray *data)
{
    if (device->bytesAvailable() < static_cast<qint64>(sizeof(PackageSize)))
        return false;

    // read payload size
    char payloadBytes[sizeof(PackageSize)];
    PackageSize *payloadSize = reinterpret_cast<PackageSize*>(&payloadBytes);
    device->read(payloadBytes, sizeof(PackageSize));

    // not enough data yet? back off ...
    if (device->bytesAvailable() < *payloadSize) {
        for (int i = sizeof(PackageSize) - 1; i >= 0; --i)
            device->ungetChar(payloadBytes[i]);
        return false;
    }

    const QByteArray payload = device->read(*payloadSize);
    int separator = payload.indexOf('\0');

    *command = payload.left(separator);
    *data = payload.right(payload.size() - separator - 1);
    return true;
}

} // namespace QInstaller
