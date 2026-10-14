/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "clickablelabel.h"

#include <QCheckBox>
#include <QMouseEvent>

using namespace QInstaller;

ClickableLabel::ClickableLabel(const QString &message, const QString &objectName, QWidget* parent)
    : QLabel(message, parent)
{
    setObjectName(objectName);
    setOpenExternalLinks(true);
    setWordWrap(true);
    setTextFormat(Qt::RichText);
    QFont clickableFont = font();
    clickableFont.setUnderline(true);
    setFont(clickableFont);
}

void ClickableLabel::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event)
    emit clicked();
}

bool ClickableLabel::event(QEvent *e)
{
    if (e->type() == QEvent::Enter)
        setCursor(Qt::PointingHandCursor);
    return QLabel::event(e);
}
