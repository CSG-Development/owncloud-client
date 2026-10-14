/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#ifndef LABELWITHPIXMAP_H
#define LABELWITHPIXMAP_H

#include "installer_global.h"

#include <QWidget>
#include <QLabel>

namespace QInstaller {

class INSTALLER_EXPORT LabelWithPixmap : public QWidget
{
    Q_OBJECT

public:
    explicit LabelWithPixmap(const QString &text, QString pixmap, QWidget *parent = nullptr);

public:
    void setWarningText(const QString &text);

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;

private:
    QLabel *m_warningText;
    QLabel *m_labelPixmap;
};

} // namespace QInstaller

#endif // LABELWITHPIXMAP_H
