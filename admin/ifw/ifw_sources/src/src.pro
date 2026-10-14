# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

TEMPLATE = subdirs
SUBDIRS += libs sdk sdk/translations
sdk.depends = libs
