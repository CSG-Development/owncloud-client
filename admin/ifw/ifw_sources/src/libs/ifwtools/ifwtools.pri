# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

INCLUDEPATH += $$PWD $$PWD/rcc

DEFINES += BUILD_SHARED_IFWTOOLS

HEADERS += $$PWD/rcc/rcc.h \
    $$PWD/rcc/qcorecmdlineargs_p.h

SOURCES += $$PWD/rcc/rcc.cpp \
    $$PWD/rcc/rccmain.cpp

HEADERS += $$PWD/ifwtools_global.h \
    $$PWD/repositorygen.h \
    $$PWD/binarycreator.h \
    $$PWD/binarydump.h

SOURCES += $$PWD/repositorygen.cpp \
    $$PWD/binarycreator.cpp \
    $$PWD/binarydump.cpp

RESOURCES += $$PWD/resources/ifwtools.qrc

