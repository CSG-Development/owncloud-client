# SPDX-License-Identifier: bzip2-1.0.6
TEMPLATE = lib
TARGET = libbz2
CONFIG += staticlib
INCLUDEPATH += $$PWD

include(../../../../installerfw.pri)
DESTDIR = $$IFW_LIB_PATH

DEFINES += _CRT_SECURE_NO_WARNINGS

SOURCES += \
    $$PWD/blocksort.c \
    $$PWD/huffman.c \
    $$PWD/crctable.c \
    $$PWD/randtable.c \
    $$PWD/compress.c \
    $$PWD/decompress.c \
    $$PWD/bzlib.c
