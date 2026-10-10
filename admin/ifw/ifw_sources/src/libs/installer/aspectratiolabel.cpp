/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "aspectratiolabel.h"

#include <QDesktopServices>
#include <QTimer>

using namespace QInstaller;

/*!
    \class QInstaller::AspectRatioLabel
    \inmodule QtInstallerFramework
    \brief The AspectRatioLabel class provides a label for displaying
        a pixmap that maintains its original aspect ratio when resized.
*/

/*!
    Constructs the label with \a parent as parent.
*/
AspectRatioLabel::AspectRatioLabel(QWidget *parent)
    : QLabel(parent)
    , m_discardMousePress(false)
{
    setMinimumSize(1, 1);
    setScaledContents(false);
    setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
}

/*!
    Sets the \a pixmap shown on the label. Setting a new pixmap clears
    the previous content.

    \note This redefines the non-virtual slot of the same signature from the
    QLabel base class, which results in non polymorphic behavior when
    called via a base class pointer.
*/
void AspectRatioLabel::setPixmap(const QPixmap &pixmap)
{
    setPixmapAndUrl(pixmap, QString());
}

/*!
    Sets the \a pixmap shown on the label and an \a url. Setting a new
    pixmap clears the previous content. When clicking the \a pixmap, \a url
    is opened in a browser. If the \a url is a reference to a file, it will
    be opened with a suitable application instead of a Web browser.
*/
void AspectRatioLabel::setPixmapAndUrl(const QPixmap &pixmap, const QString &url)
{
    m_clickableUrl = url;
    m_pixmap = pixmap;
    QLabel::setPixmap(scaledPixmap());
}

/*!
    \reimp
*/
int AspectRatioLabel::heightForWidth(int w) const
{
    return m_pixmap.isNull()
        ? height()
        : m_pixmap.height() * w / m_pixmap.width();
}

/*!
    \reimp
*/
QSize AspectRatioLabel::sizeHint() const
{
    const int w = width();
    return QSize(w, heightForWidth(w));
}

/*!
    Returns the currently set pixmap scaled to the label size,
    while preserving the original aspect ratio.
*/
QPixmap AspectRatioLabel::scaledPixmap() const
{
    if (m_pixmap.isNull())
        return QPixmap();

    return m_pixmap.scaled(size() * m_pixmap.devicePixelRatio(),
        Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

/*!
    \reimp
*/
void AspectRatioLabel::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if (!m_pixmap.isNull())
        QLabel::setPixmap(scaledPixmap());
}

/*!
    \reimp
*/
void AspectRatioLabel::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event)
    if (!m_clickableUrl.isEmpty() && !m_discardMousePress)
        QDesktopServices::openUrl(m_clickableUrl);
}

/*!
    \reimp
*/
bool AspectRatioLabel::event(QEvent *e)
{
    if (e->type() == QEvent::WindowActivate) {
        QTimer::singleShot(100, [&]() {
            m_discardMousePress = false;
        });
    } else if (e->type() == QEvent::WindowDeactivate) {
        m_discardMousePress = true;
    }
    return QLabel::event(e);
}
