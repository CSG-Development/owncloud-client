/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef QCORECMDLINEARGS_P_H
#define QCORECMDLINEARGS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

static inline QStringList qCmdLineArgs(int argc, char *argv[])
{
    QStringList args;
	for (int i = 0; i != argc; ++i)
        args += QString::fromLocal8Bit(argv[i]);
    return args;
}


QT_END_NAMESPACE

#endif // QCORECMDLINEARGS_WIN_P_H
