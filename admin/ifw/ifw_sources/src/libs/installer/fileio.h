/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef FILEIO_H
#define FILEIO_H

#include "installer_global.h"

QT_BEGIN_NAMESPACE
class QByteArray;
class QFileDevice;
class QString;
QT_END_NAMESPACE

template <typename T>
class Range;

namespace QInstaller {

qint64 INSTALLER_EXPORT retrieveInt64(QFileDevice *in);
void INSTALLER_EXPORT appendInt64(QFileDevice *out, qint64 n);

Range<qint64> INSTALLER_EXPORT retrieveInt64Range(QFileDevice *in);
void INSTALLER_EXPORT appendInt64Range(QFileDevice *out, const Range<qint64> &r);

QString INSTALLER_EXPORT retrieveString(QFileDevice *in);
void INSTALLER_EXPORT appendString(QFileDevice *out, const QString &str);

QByteArray INSTALLER_EXPORT retrieveByteArray(QFileDevice *in);
void INSTALLER_EXPORT appendByteArray(QFileDevice *out, const QByteArray &ba);

QByteArray INSTALLER_EXPORT retrieveData(QFileDevice *in, qint64 size);
void INSTALLER_EXPORT appendData(QFileDevice *out, QFileDevice *in, qint64 size);

void INSTALLER_EXPORT openForRead(QFileDevice *dev);
void INSTALLER_EXPORT openForWrite(QFileDevice *dev);
void INSTALLER_EXPORT openForAppend(QFileDevice *dev);

qint64 INSTALLER_EXPORT blockingRead(QFileDevice *in, char *buffer, qint64 size);
qint64 INSTALLER_EXPORT blockingCopy(QFileDevice *in, QFileDevice *out, qint64 size);

qint64 INSTALLER_EXPORT blockingWrite(QFileDevice *out, const QByteArray &data);
qint64 INSTALLER_EXPORT blockingWrite(QFileDevice *out, const char *data, qint64 size);

} // namespace QInstaller

#endif // FILEIO_H
