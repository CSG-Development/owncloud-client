/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#ifndef INSTALLER_GLOBAL_H
#define INSTALLER_GLOBAL_H

#include <QtCore/QtGlobal>

#ifndef QT_STATIC
#  ifdef BUILD_LIB_INSTALLER
#    define INSTALLER_EXPORT Q_DECL_EXPORT
#  else
#    define INSTALLER_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define INSTALLER_EXPORT
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
typedef uint hashValue;
#else
typedef size_t hashValue;
#endif

#endif //INSTALLER_GLOBAL_H
