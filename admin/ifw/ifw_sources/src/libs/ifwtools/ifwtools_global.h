/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef IFWTOOLS_GLOBAL_H
#define IFWTOOLS_GLOBAL_H

#include <QtCore/QtGlobal>

#ifndef QT_STATIC
#  ifdef BUILD_SHARED_IFWTOOLS
#    define IFWTOOLS_EXPORT Q_DECL_EXPORT
#  else
#    define IFWTOOLS_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define IFWTOOLS_EXPORT
#endif

#endif //IFWTOOLS_GLOBAL_H
