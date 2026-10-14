# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

TEMPLATE = subdirs
SUBDIRS += src tools
tools.depends = src

requires(!cross_compile)

include (installerfw.pri)

!minQtVersion(5, 15, 2) {
    message("Cannot build Qt Installer Framework with Qt version $${QT_VERSION}.")
    error("Use at least Qt 5.15.2.")
}
