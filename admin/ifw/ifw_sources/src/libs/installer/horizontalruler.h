/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef HORIZONTALRULER_H
#define HORIZONTALRULER_H

#include "installer_global.h"

#include <QWidget>

namespace QInstaller {

class INSTALLER_EXPORT HorizontalRuler : public QWidget
{
    Q_OBJECT
public:
    explicit HorizontalRuler(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

}

#endif // HORIZONTALRULER_H
