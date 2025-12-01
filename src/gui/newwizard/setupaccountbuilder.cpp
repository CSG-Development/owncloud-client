/*
 * Copyright (C) Fabian Müller <fmueller@owncloud.com>
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

#include "setupaccountbuilder.h"

#include "gui/accountmanager.h"
#include "networkjobs/fetchuserinfojobfactory.h"

#include <QDir>
#include <QFileInfo>

namespace CUR::Wizard {

AbstractAuthenticationStrategy::~AbstractAuthenticationStrategy() { }

QString AbstractAuthenticationStrategy::davUser()
{
    return _davUser;
}

void AbstractAuthenticationStrategy::setDavUser(const QString &user)
{
    _davUser = user;
}

HttpBasicAuthenticationStrategy::HttpBasicAuthenticationStrategy(const QString &username, const QString &password)
    : _loginUser(username)
    , _password(password)
{
}

HttpCredentialsGui *HttpBasicAuthenticationStrategy::makeCreds()
{
    return new HttpCredentialsGui(loginUser(), _password);
}

bool HttpBasicAuthenticationStrategy::isValid()
{
    return !loginUser().isEmpty() && !_password.isEmpty();
}

QString HttpBasicAuthenticationStrategy::password() const
{
    return _password;
}

QString HttpBasicAuthenticationStrategy::loginUser() const
{
    return _loginUser;
}

FetchUserInfoJobFactory HttpBasicAuthenticationStrategy::makeFetchUserInfoJobFactory(QNetworkAccessManager *nam)
{
    return FetchUserInfoJobFactory::fromBasicAuthCredentials(nam, loginUser(), _password);
}

OAuth2AuthenticationStrategy::OAuth2AuthenticationStrategy(const QString &token, const QString &refreshToken)
    : _token(token)
    , _refreshToken(refreshToken)
{
}

HttpCredentialsGui *OAuth2AuthenticationStrategy::makeCreds()
{
    Q_ASSERT(isValid());
    return new HttpCredentialsGui(davUser(), _token, _refreshToken);
}

bool OAuth2AuthenticationStrategy::isValid()
{
    return !davUser().isEmpty() && !_token.isEmpty() && !_refreshToken.isEmpty();
}

FetchUserInfoJobFactory OAuth2AuthenticationStrategy::makeFetchUserInfoJobFactory(QNetworkAccessManager *nam)
{
    return FetchUserInfoJobFactory::fromOAuth2Credentials(nam, _token);
}

SetupAccountBuilder::SetupAccountBuilder() = default;

void SetupAccountBuilder::setServerUrl(const QUrl &serverUrl, DetermineAuthTypeJob::AuthType authType)
{
    _serverUrl = serverUrl;
    _authType = authType;

    // to not keep credentials longer than necessary, we purge them whenever the URL is set
    // for this reason, we also don't insert already-known credentials on the credentials pages when switching to them
    _authenticationStrategy.reset();
}

QUrl SetupAccountBuilder::serverUrl() const
{
    return _serverUrl;
}

void SetupAccountBuilder::setDevice(const Device& dev)
{
    _device = dev;
}

Device SetupAccountBuilder::device() const
{
    return _device;
}

DetermineAuthTypeJob::AuthType SetupAccountBuilder::authType()
{
    return _authType;
}

void SetupAccountBuilder::setLegacyWebFingerUsername(const QString &username)
{
    _legacyWebFingerUsername = username;
}

AccountPtr SetupAccountBuilder::build()
{
    auto newAccountPtr = Account::create(QUuid::createUuid());

    Q_ASSERT(!_serverUrl.isEmpty() && _serverUrl.isValid());
    //newAccountPtr->setUrl(_serverUrl);
    newAccountPtr->setDevice(_device);

    //if (!_webFingerSelectedInstance.isEmpty()) {
    //    Q_ASSERT(_serverUrl.isValid());
    //    newAccountPtr->setUrl(_webFingerSelectedInstance);
    //}

    Q_ASSERT(hasValidCredentials());

    // TODO: perhaps _authenticationStrategy->setUpAccountPtr(...) would be more elegant? no need for getters then
    newAccountPtr->setDavUser(_authenticationStrategy->davUser());
    newAccountPtr->setCredentials(_authenticationStrategy->makeCreds());
    newAccountPtr->setDavDisplayName(_displayName);
    newAccountPtr->addApprovedCerts({ _customTrustedCaCertificates.begin(), _customTrustedCaCertificates.end() });

    if (!_defaultSyncTargetDir.isEmpty()) {
        newAccountPtr->setDefaultSyncRoot(_defaultSyncTargetDir);
        if (!QFileInfo::exists(_defaultSyncTargetDir)) {
            OC_ASSERT(QDir().mkpath(_defaultSyncTargetDir));
        }
    }

    return newAccountPtr;
}

bool SetupAccountBuilder::hasValidCredentials() const
{
    if (_authenticationStrategy == nullptr) {
        return false;
    }

    return _authenticationStrategy->isValid();
}

QString SetupAccountBuilder::displayName() const
{
    return _displayName;
}

void SetupAccountBuilder::setDisplayName(const QString &displayName)
{
    _displayName = displayName;
}

void SetupAccountBuilder::setAuthenticationStrategy(AbstractAuthenticationStrategy *strategy)
{
    _authenticationStrategy.reset(strategy);
}

void SetupAccountBuilder::addCustomTrustedCaCertificate(const QSslCertificate &customTrustedCaCertificate)
{
    _customTrustedCaCertificates.insert(customTrustedCaCertificate);
}

void SetupAccountBuilder::clearCustomTrustedCaCertificates()
{
    _customTrustedCaCertificates.clear();
}

AbstractAuthenticationStrategy *SetupAccountBuilder::authenticationStrategy() const
{
    return _authenticationStrategy.get();
}

void SetupAccountBuilder::setSyncTargetDir(const QString &syncTargetDir)
{
    _defaultSyncTargetDir = syncTargetDir;
}

QString SetupAccountBuilder::syncTargetDir() const
{
    return _defaultSyncTargetDir;
}

QString SetupAccountBuilder::legacyWebFingerUsername() const
{
    return _legacyWebFingerUsername;
}

void SetupAccountBuilder::setLegacyWebFingerServerUrl(const QUrl &webFingerServerUrl)
{
    _legacyWebFingerServerUrl = webFingerServerUrl;
}

QUrl SetupAccountBuilder::legacyWebFingerServerUrl() const
{
    return _legacyWebFingerServerUrl;
}

void SetupAccountBuilder::setDynamicRegistrationData(const QVariantMap &dynamicRegistrationData)
{
    _dynamicRegistrationData = dynamicRegistrationData;
}

QVariantMap SetupAccountBuilder::dynamicRegistrationData() const
{
    return _dynamicRegistrationData;
}

void SetupAccountBuilder::setWebFingerAuthenticationServerUrl(const QUrl &url)
{
    _webFingerAuthenticationServerUrl = url;
    _authType = DetermineAuthTypeJob::AuthType::OAuth;
}

QUrl SetupAccountBuilder::webFingerAuthenticationServerUrl() const
{
    return _webFingerAuthenticationServerUrl;
}

void SetupAccountBuilder::setWebFingerInstances(const QVector<QUrl> &instancesList)
{
    _webFingerInstances = instancesList;
}

QVector<QUrl> SetupAccountBuilder::webFingerInstances() const
{
    return _webFingerInstances;
}

void SetupAccountBuilder::setWebFingerSelectedInstance(const QUrl &instance)
{
    _webFingerSelectedInstance = instance;
}

QUrl SetupAccountBuilder::webFingerSelectedInstance() const
{
    return _webFingerSelectedInstance;
}
}
