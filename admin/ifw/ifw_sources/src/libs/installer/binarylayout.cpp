/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

/*!
    \class QInstaller::BinaryLayout
    \inmodule QtInstallerFramework
    \brief The BinaryLayout class describes the binary content appended to a file.

    Explanation of the binary content at the end of the installer or the separate data file:

    \code

    Meta data entry [1 ... n]
    [Format]
        Plain data (QResource)
    [Format]
    ----------------------------------------------------------
    Operation count (qint64)
    Operation entry [1 ... n]
    [Format]
        Name (qint64, QString)
        XML (qint64, QString)
    [Format]
    Operation count (qint64)
    ----------------------------------------------------------
    Collection count
    Collection data entry [1 ... n]
    [Format]
        Archive count (qint64),
        Name entry [1 ... n]
        [Format]
            Name (qint64, QByteArray),
            Offset (qint64),
            Length (qint64),
        [Format]
        Archive data entry [1 ... n]
        [Format]
            Plain data
        [Format]
    [Format]
    ----------------------------------------------------------
    Collection count (qint64)
    Collection index entry [1 ... n]
    [Format]
        Name (qint64, QByteArray)
        Offset (qint64)
        Length (qint64)
    [Format]
    Collection count (qint64)
    ----------------------------------------------------------
    Collection index block [Offset (qint64)]
    Collection index block [Length (qint64)]
    ----------------------------------------------------------
    Resource segments [1 ... n]
    [Format]
        Offset (qint64)
        Length (qint64)
    [Format]
    ----------------------------------------------------------
    Operations information block [Offset (qint64)]
    Operations information block [Length (qint64)]
    ----------------------------------------------------------
    Meta data count (qint64)
    ----------------------------------------------------------
    Binary content size [Including Marker and Cookie (qint64)]
    ----------------------------------------------------------
    Magic marker (qint64)
    Magic cookie (qint64)

    \endcode
*/
