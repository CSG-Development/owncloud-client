# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

TEMPLATE = subdirs

include(../../../installerfw.pri)

SUBDIRS += libarchive
win32: SUBDIRS += zlib bzip2 lzma
