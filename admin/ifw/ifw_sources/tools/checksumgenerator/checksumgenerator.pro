# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

TEMPLATE = app
INCLUDEPATH += . ..
TARGET = checksumgenerator

include(../../installerfw.pri)

QT -= gui
CONFIG += console
CONFIG -= import_plugins

SOURCES += main.cpp
