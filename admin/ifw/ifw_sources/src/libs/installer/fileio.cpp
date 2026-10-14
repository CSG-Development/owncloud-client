/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "fileio.h"

#include "errors.h"
#include "range.h"

#include <QCoreApplication>
#include <QByteArray>
#include <QDir>
#include <QFileDevice>
#include <QString>

/*!
    \internal
*/
qint64 QInstaller::retrieveInt64(QFileDevice *in)
{
    qint64 n = 0;
    QInstaller::blockingRead(in, reinterpret_cast<char*>(&n), sizeof(n));
    return n;
}

/*!
    \internal
*/
void QInstaller::appendInt64(QFileDevice *out, qint64 n)
{
    QInstaller::blockingWrite(out, reinterpret_cast<const char*>(&n), sizeof(n));
}

/*!
    \internal
*/
Range<qint64> QInstaller::retrieveInt64Range(QFileDevice *in)
{
    const quint64 start = QInstaller::retrieveInt64(in);
    const quint64 length = QInstaller::retrieveInt64(in);
    return Range<qint64>::fromStartAndLength(start, length);
}

/*!
    \internal
*/
void QInstaller::appendInt64Range(QFileDevice *out, const Range<qint64> &r)
{
    QInstaller::appendInt64(out, r.start());
    QInstaller::appendInt64(out, r.length());
}

/*!
    \internal
*/
QString QInstaller::retrieveString(QFileDevice *in)
{
    return QString::fromUtf8(QInstaller::retrieveByteArray(in));
}

/*!
    \internal
*/
void QInstaller::appendString(QFileDevice *out, const QString &str)
{
    QInstaller::appendByteArray(out, str.toUtf8());
}

/*!
    \internal
*/
QByteArray QInstaller::retrieveByteArray(QFileDevice *in)
{
    QByteArray ba(QInstaller::retrieveInt64(in), '\0');
    QInstaller::blockingRead(in, ba.data(), ba.size());
    return ba;
}

/*!
    \internal
*/
void QInstaller::appendByteArray(QFileDevice *out, const QByteArray &ba)
{
    QInstaller::appendInt64(out, ba.size());
    QInstaller::blockingWrite(out, ba.constData(), ba.size());
}

/*!
    \internal
*/
QByteArray QInstaller::retrieveData(QFileDevice *in, qint64 size)
{
    QByteArray ba(size, '\0');
    QInstaller::blockingRead(in, ba.data(), size);
    return ba;
}

/*!
    \internal
*/
void QInstaller::appendData(QFileDevice *out, QFileDevice *in, qint64 size)
{
    Q_ASSERT(!in->isSequential());
    QInstaller::blockingCopy(in, out, size);
}

/*!
    \internal
*/
void QInstaller::openForRead(QFileDevice *dev)
{
    Q_ASSERT(dev);
    if (!dev->open(QIODevice::ReadOnly)) {
        throw Error(QCoreApplication::translate("QInstaller",
            "Cannot open file \"%1\" for reading: %2").arg(
                        QDir::toNativeSeparators(dev->fileName()), dev->errorString()));
    }
}

/*!
    \internal
*/
void QInstaller::openForWrite(QFileDevice *dev)
{
    Q_ASSERT(dev);
    if (!dev->open(QIODevice::WriteOnly)) {
        throw Error(QCoreApplication::translate("QInstaller",
            "Cannot open file \"%1\" for writing: %2").arg(
                        QDir::toNativeSeparators(dev->fileName()), dev->errorString()));
    }
}

/*!
    \internal
*/
void QInstaller::openForAppend(QFileDevice *dev)
{
    Q_ASSERT(dev);
    if (!dev->open(QIODevice::WriteOnly | QIODevice::Append)) {
        throw Error(QCoreApplication::translate("QInstaller",
            "Cannot open file \"%1\" for writing: %2").arg(
                        QDir::toNativeSeparators(dev->fileName()), dev->errorString()));
    }
}

/*!
    \internal
*/
qint64 QInstaller::blockingRead(QFileDevice *in, char *buffer, qint64 size)
{
    if (in->atEnd())
        return 0;
    qint64 left = size;
    while (left > 0) {
        const qint64 n = in->read(buffer, left);
        if (n < 0) {
            throw Error(QCoreApplication::translate("QInstaller",
                "Read failed after %1 bytes: %2").arg(QString::number(size - left),
                in->errorString()));
        }
        left -= n;
        buffer += n;
    }
    return size;
}

/*!
    \internal
*/
qint64 QInstaller::blockingCopy(QFileDevice *in, QFileDevice *out, qint64 size)
{
    static const qint64 blockSize = 4096;
    QByteArray ba(blockSize, '\0');
    qint64 actual = qMin(blockSize, size);
    while (actual > 0) {
        try {
            QInstaller::blockingRead(in, ba.data(), actual);
            QInstaller::blockingWrite(out, ba.constData(), actual);
            size -= actual;
            actual = qMin(blockSize, size);
        } catch (const Error &error) {
            throw Error(QCoreApplication::translate("QInstaller", "Copy failed: %1")
                .arg(error.message()));
        }
    }
    return size;
}

/*!
    \internal
*/
qint64 QInstaller::blockingWrite(QFileDevice *out, const QByteArray &data)
{
    return QInstaller::blockingWrite(out, data.constData(), data.size());
}

/*!
    \internal
*/
qint64 QInstaller::blockingWrite(QFileDevice *out, const char *data, qint64 size)
{
    qint64 left = size;
    while (left > 0) {
        const qint64 n = out->write(data, left);
        if (n < 0) {
            throw Error(QCoreApplication::translate("QInstaller",
                "Write failed after %1 bytes: %2").arg(QString::number(size - left),
                out->errorString()));
        }
        left -= n;
    }
    return size;
}
