/*
 * Copyright (C) by Klaas Freitag <freitag@owncloud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "curatortheme.h"

#include <QString>
#include <QVariant>
#include <QPixmap>
#include <QIcon>
#include <QCoreApplication>

#include "common/utility.h"

namespace CUR {

CuratorTheme::CuratorTheme()
    : Theme()
{
}

QColor CuratorTheme::wizardHeaderBackgroundColor() const
{
    return QColor(4, 30, 66);
}

QColor CuratorTheme::wizardHeaderTitleColor() const
{
    return Qt::white;
}

QIcon CuratorTheme::wizardHeaderLogo() const
{
    return themeUniversalIcon(QStringLiteral("wizard_logo"));
}

QIcon CuratorTheme::aboutIcon() const
{
    return themeUniversalIcon(QStringLiteral("oc-image-about"));
}
}
