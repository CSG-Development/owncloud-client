/*
 * Copyright (C) by Hannah von Reth <hannah.vonreth@owncloud.com>
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

#include "resources.h"

#include <QPalette>
#include <QDebug>
#include <QFile>

using namespace CUR;
using namespace Resources;

bool CUR::Resources::isUsingDarkTheme()
{
    // TODO: replace by a command line switch
    static bool forceDark = qEnvironmentVariableIntValue("CURATOR_FORCE_DARK_MODE") != 0;
    return forceDark || QPalette().base().color().lightnessF() <= 0.5;
}

QIcon CUR::Resources::getCoreIcon(const QString &icon_name)
{
    if (icon_name.isEmpty()) {
        return {};
    }
    const QString theme = Resources::isUsingDarkTheme() ? QStringLiteral("dark") : QStringLiteral("light");
    const QString path_svg = QStringLiteral(":/client/resources/%1/%2.svg").arg(theme, icon_name);
    if (QFile::exists(path_svg)) {
        const QIcon icon(path_svg);
        return icon;
    }
    const QString path_png = QStringLiteral(":/client/resources/%1/%2.png").arg(theme, icon_name);
    if (QFile::exists(path_png)) {
        const QIcon icon(path_png);
        return icon;
    }

    qWarning() << "Unable to load icon" << path_png << "or" << path_svg;
    return {};
}
