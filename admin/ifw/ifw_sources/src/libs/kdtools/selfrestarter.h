/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef SELFRESTARTER_H
#define SELFRESTARTER_H

#include "kdtoolsglobal.h"

class KDTOOLS_EXPORT SelfRestarter
{
public:
    SelfRestarter(int argc, char *argv[]);
    ~SelfRestarter();

    static bool restartOnQuit();
    static void setRestartOnQuit(bool restart);

private:
    Q_DISABLE_COPY(SelfRestarter)
    class Private;
    Private *d;
};

#endif // SELFRESTARTER_H
