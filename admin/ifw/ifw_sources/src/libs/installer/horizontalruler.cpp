/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "horizontalruler.h"

#include <QStylePainter>
#include <QStyleOption>

using namespace QInstaller;

HorizontalRuler::HorizontalRuler(QWidget *parent)
    : QWidget{parent}
{
    setFixedHeight(4);
    setObjectName(QLatin1String("HorizontalRuler"));
}

void HorizontalRuler::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QStylePainter painter(this);
    int x = width() - 2;
    int y = height() - 2;
    const QPalette &pal = palette();
    painter.setPen(pal.mid().color());
    painter.drawLine(0, y, x, y);
    painter.setPen(pal.base().color());
    painter.drawPoint(x + 1, y);
    painter.drawLine(0, y + 1, x + 1, y + 1);
}
