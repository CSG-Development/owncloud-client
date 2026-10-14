/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "labelwithpixmap.h"

#include "fileutils.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStyleOption>
#include <QPainter>

using namespace QInstaller;

LabelWithPixmap::LabelWithPixmap(const QString &text, QString pixmap, QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QLatin1String("LabelWithPixmap"));
    QHBoxLayout *layout = new QHBoxLayout(this);

    m_labelPixmap = new QLabel(this);
    m_labelPixmap->setObjectName(QLatin1String("PixmapLabel"));
    QInstaller::replaceHighDpiImage(pixmap);
    m_labelPixmap->setPixmap(QPixmap(pixmap));
    layout->addWidget(m_labelPixmap);

    m_warningText = new QLabel(text, this);
    m_warningText->setWordWrap(true);
    m_warningText->setObjectName(QLatin1String("LabelText"));

    layout->addWidget(m_warningText, 1);
}

void LabelWithPixmap::setWarningText(const QString &text)
{
    m_warningText->setText(text);
}

void LabelWithPixmap::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

QSize LabelWithPixmap::sizeHint() const
{
    int width = 10; // includes padding
    int height = 0;

    if (!m_warningText->text().isEmpty()) {
        QFontMetrics metrics(m_warningText->font());
        width += metrics.horizontalAdvance(m_warningText->text());
        height = metrics.height(); // Single line height
    }

    if (!m_labelPixmap->pixmap().isNull()) {
        QSize pixmapSize = m_labelPixmap->pixmap().size();
        width += pixmapSize.width();
        height = qMax(height, pixmapSize.height());
    }

    width += layout()->spacing();
    QMargins margins = layout()->contentsMargins();
    width += margins.left() + margins.right();
    height += margins.top() + margins.bottom();

    return QSize(width, height);
}
