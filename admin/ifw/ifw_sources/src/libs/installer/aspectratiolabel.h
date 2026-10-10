/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef ASPECTRATIOLABEL_H
#define ASPECTRATIOLABEL_H

#include "installer_global.h"

#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>

namespace QInstaller {

class INSTALLER_EXPORT AspectRatioLabel : public QLabel
{
    Q_OBJECT

public:
    explicit AspectRatioLabel(QWidget *parent = nullptr);

    int heightForWidth(int w) const override;
    QSize sizeHint() const override;

public slots:
    void setPixmap(const QPixmap &pixmap);
    void setPixmapAndUrl (const QPixmap &pixmap, const QString &url);
    void resizeEvent(QResizeEvent *event) override;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    bool event(QEvent *e) override;

private:
    QPixmap scaledPixmap() const;

private:
    QPixmap m_pixmap;
    QString m_clickableUrl;
    bool m_discardMousePress;
};

} // namespace QInstaller

#endif // ASPECTRATIOLABEL_H
