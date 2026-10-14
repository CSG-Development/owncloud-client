/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef KDTOOLSGLOBAL_H
#define KDTOOLSGLOBAL_H

#include <QtCore/QtGlobal>

#ifndef QT_STATIC
#  ifdef BUILD_SHARED_KDTOOLS
#    define KDTOOLS_EXPORT Q_DECL_EXPORT
#  else
#    define KDTOOLS_EXPORT Q_DECL_IMPORT
#  endif
#else // KDTOOLS_SHARED
#  define KDTOOLS_EXPORT
#endif // KDTOOLS_SHARED

#endif // KDTOOLSGLOBAL_H

