/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef PACKAGEMANAGERPROXYFACTORY_H
#define PACKAGEMANAGERPROXYFACTORY_H

#include "filedownloaderfactory.h"

namespace QInstaller {

class PackageManagerCore;

struct ProxyCredential {
    QString host;
    QString user;
    QString password;
    int port;
};

class PackageManagerProxyFactory : public KDUpdater::FileDownloaderProxyFactory
{
public:
    explicit PackageManagerProxyFactory(const PackageManagerCore *const core);

    PackageManagerProxyFactory *clone() const override;
    QList<QNetworkProxy> queryProxy(const QNetworkProxyQuery &query = QNetworkProxyQuery()) override;

    void setProxyCredentials(const QNetworkProxy &proxy, const QString &user, const QString &password);
    bool askProxyCredentials(const QNetworkProxy &proxy);

private:

    QList<ProxyCredential> m_proxyCredentials;
    const PackageManagerCore *const m_core;
};

}   // QInstaller

#endif // PACKAGEMANAGERPROXYFACTORY_H
