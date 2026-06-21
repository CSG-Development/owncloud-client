/*
 * Copyright (C) by the PersonalCloud developers
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

#include "proxycredentials.h"

#include "configfile.h"
#include "credentialmanager.h"

#include <QCoreApplication>
#include <QLoggingCategory>
#include <QPointer>
#include <QSettings>
#include <qt6keychain/keychain.h>

namespace APP
{

Q_LOGGING_CATEGORY(lcProxyCredentials, "sync.proxycredentials", QtInfoMsg)

namespace
{
QString legacyProxyPasswordKey()
{
    return QStringLiteral("Proxy/pass");
}
}   // namespace

QString ProxyCredentials::passwordKey()
{
    return QStringLiteral("Proxy/Password");
}

ProxyCredentials::ProxyCredentials(QObject *parent)
    : QObject(parent)
    , _manager(new CredentialManager(this))
{
}

QKeychain::Job *ProxyCredentials::startLegacyMigration(QString *password)
{
    const QString legacyPassword = QString::fromUtf8(QByteArray::fromBase64(ConfigFile::makeQSettings().value(legacyProxyPasswordKey()).toByteArray()));
    if (legacyPassword.isEmpty()) {
        return nullptr;
    }

    qCWarning(lcProxyCredentials) << "Migrating legacy proxy password to keychain";
    if (password) {
        *password = legacyPassword;
    }

    auto *writeJob = _manager->set(passwordKey(), legacyPassword);
    connect(writeJob, &QKeychain::Job::finished, this, [writeJob] {
        if (writeJob->error() == QKeychain::NoError) {
            QSettings settings = ConfigFile::makeQSettings();
            settings.remove(legacyProxyPasswordKey());
            settings.sync();
        }
        else {
            qCWarning(lcProxyCredentials) << "Failed to migrate proxy password to keychain, keeping legacy value:" << writeJob->errorString();
        }
    });
    return writeJob;
}

void ProxyCredentials::removeLegacyPassword()
{
    QSettings settings = ConfigFile::makeQSettings();
    if (settings.contains(legacyProxyPasswordKey())) {
        qCInfo(lcProxyCredentials) << "Removing unused legacy proxy password from config";
        settings.remove(legacyProxyPasswordKey());
        settings.sync();
    }
}

void ProxyCredentials::load(QObject *context, const std::function<void(const QString &password)> &callback)
{
    auto *self = new ProxyCredentials(qApp);
    const QPointer<QObject> contextGuard(context);

    QString legacyPassword;
    if (auto *writeJob = self->startLegacyMigration(&legacyPassword)) {
        if (contextGuard) {
            callback(legacyPassword);
        }
        connect(writeJob, &QKeychain::Job::finished, self, [self] { self->deleteLater(); });
        return;
    }

    auto *job = self->_manager->get(passwordKey());
    connect(job, &CredentialJob::finished, self, [self, job, contextGuard, callback] {
        if (job->error() == QKeychain::NoError || job->error() == QKeychain::EntryNotFound) {
            if (contextGuard) {
                callback(job->data().toString());
            }
        }
        else {
            qCWarning(lcProxyCredentials) << "Failed to read proxy password from keychain, leaving proxy unchanged:" << job->errorString();
        }
        self->deleteLater();
    });
}

void ProxyCredentials::store(const QString &password)
{
    auto *self = new ProxyCredentials(qApp);
    auto *job = self->_manager->set(passwordKey(), password);
    connect(job, &QKeychain::Job::finished, self, [self] { self->deleteLater(); });
}

void ProxyCredentials::remove()
{
    auto *self = new ProxyCredentials(qApp);
    if (!self->_manager->contains(passwordKey())) {
        self->deleteLater();
        return;
    }
    auto *job = self->_manager->remove(passwordKey());
    connect(job, &QKeychain::Job::finished, self, [self] { self->deleteLater(); });
}

}   // namespace APP
