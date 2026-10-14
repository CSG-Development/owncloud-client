/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef BINARYLAYOUT_H
#define BINARYLAYOUT_H

#include "range.h"

#include <QVector>

namespace QInstaller {

struct BinaryLayout
{
    qint64 endOfExectuable;

    QVector<Range<qint64> > metaResourceSegments;

    Range<qint64> metaResourcesSegment;
    Range<qint64> operationsSegment;
    Range<qint64> resourceCollectionsSegment;

    qint64 binaryContentSize;
    qint64 magicMarker;
    quint64 magicCookie;

    qint64 endOfBinaryContent;
};

} // namespace QInstaller

#endif // BINARYLAYOUT
