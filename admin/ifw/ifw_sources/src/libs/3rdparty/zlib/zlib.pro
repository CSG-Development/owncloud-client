# SPDX-License-Identifier: Zlib
TEMPLATE = lib
TARGET = zlib
CONFIG += staticlib
INCLUDEPATH += $$PWD

include(../../../../installerfw.pri)
DESTDIR = $$IFW_LIB_PATH

DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE

SOURCES += \
    $$PWD/adler32.c \
    $$PWD/compress.c \
    $$PWD/crc32.c \
    $$PWD/deflate.c \
    $$PWD/gzclose.c \
    $$PWD/gzlib.c \
    $$PWD/gzread.c \
    $$PWD/gzwrite.c \
    $$PWD/infback.c \
    $$PWD/inffast.c \
    $$PWD/inflate.c \
    $$PWD/inftrees.c \
    $$PWD/trees.c \
    $$PWD/uncompr.c \
    $$PWD/zutil.c
