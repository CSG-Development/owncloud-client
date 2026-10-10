/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "packagemanagerproxyfactory.h"

#include "packagemanagercore.h"
#include "settings.h"
#include "globals.h"
#include "proxycredentialsdialog.h"

#include <QNetworkProxy>

namespace QInstaller {

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::PackageManagerProxyFactory
    \internal
*/

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::ProxyCredential
    \internal
*/

PackageManagerProxyFactory::PackageManagerProxyFactory(const PackageManagerCore *const core)
    : m_core(core)
{
}

PackageManagerProxyFactory *PackageManagerProxyFactory::clone() const
{
    PackageManagerProxyFactory *factory = new PackageManagerProxyFactory(m_core);
    factory->m_proxyCredentials = m_proxyCredentials;
    return factory;
}

struct FindProxyCredential {
    FindProxyCredential(const QString &host, int port) : host(host), port(port) {}

    bool operator()(const ProxyCredential &c) { return c.host == host && c.port == port; }
private:
    QString host;
    int port;
};

QList<QNetworkProxy> PackageManagerProxyFactory::queryProxy(const QNetworkProxyQuery &query)
{
    const Settings &settings = m_core->settings();
    QList<QNetworkProxy> list;

    if (settings.proxyType() == Settings::SystemProxy) {
        QList<QNetworkProxy> systemProxies = systemProxyForQuery(query);

        auto proxyIter = systemProxies.begin();
        for (; proxyIter != systemProxies.end(); ++proxyIter) {
            QNetworkProxy &proxy = *proxyIter;
            auto p = std::find_if(m_proxyCredentials.constBegin(), m_proxyCredentials.constEnd(),
                                  FindProxyCredential(proxy.hostName(), proxy.port()));
            if (p != m_proxyCredentials.constEnd()) {
                proxy.setUser(p->user);
                proxy.setPassword(p->password);
            }
        }
        return systemProxies;
    }

    if ((settings.proxyType() == Settings::NoProxy))
        return list << QNetworkProxy(QNetworkProxy::NoProxy);

    if (query.queryType() == QNetworkProxyQuery::UrlRequest) {
        QNetworkProxy proxy;
        if (query.url().scheme() == QLatin1String("ftp")) {
            proxy = settings.ftpProxy();
        } else if (query.url().scheme() == QLatin1String("http")
                 || query.url().scheme() == QLatin1String("https")) {
            proxy = settings.httpProxy();
        }


        auto p = std::find_if(m_proxyCredentials.constBegin(), m_proxyCredentials.constEnd(),
                              FindProxyCredential(proxy.hostName(), proxy.port()));
        if (p != m_proxyCredentials.constEnd()) {
            proxy.setUser(p->user);
            proxy.setPassword(p->password);
        }
        return list << proxy;
    }
    return list << QNetworkProxy(QNetworkProxy::DefaultProxy);
}

void PackageManagerProxyFactory::setProxyCredentials(const QNetworkProxy &proxy,
                                                     const QString &user,
                                                     const QString &password)
{
    auto p = std::find_if(m_proxyCredentials.begin(), m_proxyCredentials.end(),
                          FindProxyCredential(proxy.hostName(), proxy.port()));

    if (p == m_proxyCredentials.end()) {
        ProxyCredential proxyCredential;
        proxyCredential.host = proxy.hostName();
        proxyCredential.port = proxy.port();
        proxyCredential.user = user;
        proxyCredential.password = password;
        m_proxyCredentials.append(proxyCredential);
    } else {
        p->user = user;
        p->password = password;
    }
}

bool PackageManagerProxyFactory::askProxyCredentials(const QNetworkProxy &proxy)
{
    QString username;
    QString password;
    if (m_core->isCommandLineInstance()) {
        qCDebug(QInstaller::lcInstallerInstallLog).noquote() << QString::fromLatin1("The proxy %1:%2 requires a username and password").arg(proxy.hostName(), proxy.port());
        askForCredentials(&username, &password, QLatin1String("Username: "), QLatin1String("Password: "));
    } else {
        ProxyCredentialsDialog proxyCredentials(proxy);
        if (proxyCredentials.exec() == QDialog::Accepted) {
            username = proxyCredentials.userName();
            password = proxyCredentials.password();
        }
    }
    if (!username.isEmpty()) {
        qCDebug(QInstaller::lcInstallerInstallLog) << "Retrying with new credentials ...";
        setProxyCredentials(proxy, username, password);
        return true;
    } else {
        return false;
    }
}

}   // QInstaller
