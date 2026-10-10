/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef BINARYDUMP_H
#define BINARYDUMP_H

#include <binaryformat.h>

namespace QInstallerTools {

class BinaryDump
{
    Q_DISABLE_COPY(BinaryDump)

public:
    BinaryDump() {}
    int dump(const QInstaller::ResourceCollectionManager &manager, const QString &target);
    static void initializeBinaryDump(qint64 &magicMarker,
                                    QList<QInstaller::OperationBlob> &operations,
                                    QInstaller::ResourceCollectionManager &manager,
                                    QVector<QByteArray> &resourceMappings,
                                    quint64 cookie, const QString path);
};

} // namespace QInstallerTools

#endif // BINARYDUMP_H
