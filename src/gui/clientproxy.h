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

#ifndef CLIENTPROXY_H
#define CLIENTPROXY_H

#include "csync.h"

#include "common/utility.h"

#include <QNetworkProxy>
#include <QObject>
#include <QRunnable>
#include <QUrl>

namespace APP
{

class ConfigFile;

/**
 * @brief The ClientProxy class
 * @ingroup gui
 */
namespace ClientProxy
{
bool isUsingSystemDefault();
void lookupSystemProxyAsync(const QUrl &url, QObject *dst, const char *slot);

void applyProxy(const QString &password);
void applyProxyAndRefresh(const QString &password);

quint64 bumpProxyGeneration();
quint64 proxyGeneration();

QString printQNetworkProxy(const QNetworkProxy &proxy);
};   // namespace ClientProxy

class SystemProxyRunnable : public QObject, public QRunnable
{
    Q_OBJECT
public:
    SystemProxyRunnable(const QUrl &url);
    void run() override;
signals:
    void systemProxyLookedUp(const QNetworkProxy &url);

private:
    QUrl _url;
};
}   // namespace APP

#endif   // CLIENTPROXY_H
