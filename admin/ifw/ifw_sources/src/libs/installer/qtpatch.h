/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef QTPATCH_H
#define QTPATCH_H

#include "installer_global.h"
#include <QString>
#include <QByteArray>
#include <QHash>
#include <QFile>

namespace QtPatch {

QHash<QString, QByteArray> INSTALLER_EXPORT readQmakeOutput(const QByteArray &data);
QHash<QString, QByteArray> INSTALLER_EXPORT qmakeValues(const QString &qmakePath, QByteArray *qmakeOutput);

bool INSTALLER_EXPORT patchBinaryFile(const QString &fileName,
                                      const QByteArray &oldQtPath,
                                      const QByteArray &newQtPath );

bool INSTALLER_EXPORT patchBinaryFile(QIODevice *device,
                                      const QByteArray &oldQtPath,
                                      const QByteArray &newQtPath );

bool INSTALLER_EXPORT patchTextFile(const QString &fileName,
                                    const QHash<QByteArray, QByteArray> &searchReplacePairs);
bool INSTALLER_EXPORT openFileForPatching(QFile *file);

}

#endif // QTPATCH_H
