/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef SPACEWIDGET_H
#define SPACEWIDGET_H

#include <QWidget>
#include <QLabel>

#include "packagemanagercore.h"

namespace QInstaller {

class LabelWithPixmap;

class INSTALLER_EXPORT SpaceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SpaceWidget(PackageManagerCore *core, bool showSpaceExceedWidget = false, QWidget *parent = nullptr);

public:
    Q_INVOKABLE void updateSpaceRequiredText();

public Q_SLOTS:
    void installDirectoryChanged(const QString &newDirectory);
    void availableSpaceChanged(const PackageManagerCore::SpaceInfo spaceStatus);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    PackageManagerCore *m_core;
    QLabel *m_spaceRequiredLabel;
    QLabel *m_spaceAvailableLabel;
    LabelWithPixmap *m_noSpaceAvailableLabel;
    LabelWithPixmap *m_spaceRecommendationLabel;
    bool m_showSpaceExceedWidget;
};

}

#endif // SPACEWIDGET_H
