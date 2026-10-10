/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>

namespace QInstaller {

class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickableLabel(const QString &message, const QString &objectName, QWidget* parent = Q_NULLPTR);
    ~ClickableLabel() {};

signals:
    void clicked();

protected:
    bool event(QEvent *e) override;
    void mousePressEvent(QMouseEvent* event) override;
};

}

#endif // CLICKABLELABEL_H
