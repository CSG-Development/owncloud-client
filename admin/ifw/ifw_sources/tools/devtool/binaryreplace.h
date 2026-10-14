/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef BINARYREPLACE_H
#define BINARYREPLACE_H

#include <binarylayout.h>

class BinaryReplace
{
    Q_DISABLE_COPY(BinaryReplace)

public:
    BinaryReplace(const QInstaller::BinaryLayout &layout);
    int replace(const QString &source, const QString &target);

private:
    QInstaller::BinaryLayout m_binaryLayout;
};

#endif // BINARYREPLACE_H
