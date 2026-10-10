# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

TEMPLATE = aux

include(../../../installerfw.pri)

!exists($$LUPDATE): return()

IB_TRANSLATIONS = $$files($$PWD/*_??.ts)
IB_TRANSLATIONS -= $$PWD/ifw_en.ts

wd = $$toNativeSeparators($$IFW_SOURCE_TREE)
sources = src
lupdate_opts = -locations relative -no-ui-lines -no-sort -no-obsolete

IB_ALL_TRANSLATIONS = $$IB_TRANSLATIONS $$PWD/ifw_untranslated.ts
for(file, IB_ALL_TRANSLATIONS) {
    lang = $$replace(file, .*_([^/]*)\\.ts, \\1)
    v = ts-$${lang}.commands
    $$v = cd $$wd && $$LUPDATE $$lupdate_opts $$sources -ts $$file
    QMAKE_EXTRA_TARGETS += ts-$$lang
}
ts-all.commands = cd $$wd && $$LUPDATE $$lupdate_opts $$sources -ts $$IB_ALL_TRANSLATIONS
QMAKE_EXTRA_TARGETS += ts-all
