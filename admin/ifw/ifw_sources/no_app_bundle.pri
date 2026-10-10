# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

!isEmpty(NO_APP_BUNDLE_PRI_INCLUDED) {
    error("no_app_bundle.pri already included")
}
NO_APP_BUNDLE_PRI_INCLUDED = 1

equals(TEMPLATE, app):CONFIG -= app_bundle
