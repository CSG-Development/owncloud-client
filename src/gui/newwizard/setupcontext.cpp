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

#include "setupcontext.h"
#include "setupwidget.h"

namespace APP::Wizard {

SetupContext::SetupContext(SettingsDialog *windowParent, QObject *parent)
    : QObject(parent)
    , _window(new SetupWidget(windowParent))
{
    resetAccessManager();
}

SetupContext::~SetupContext()
{
    if (_window) {
        _window->deleteLater();
    }
    _accessManager->deleteLater();
}

AccessManager *SetupContext::resetAccessManager()
{
    if (_accessManager != nullptr) {
        _accessManager->deleteLater();
    }

    _accessManager = new AccessManager(this);
    return _accessManager;
}

SetupWidget *SetupContext::window() const
{
    return _window;
}

SetupAccountBuilder &SetupContext::accountBuilder()
{
    return _accountBuilder;
}

AccessManager *SetupContext::accessManager() const
{
    return _accessManager;
}

void SetupContext::resetAccountBuilder()
{
    _accountBuilder = {};
}

CoreJob *SetupContext::startFetchUserInfoJob(QObject *parent) const
{
    const QUrl serverUrl = [this]() {
        const QUrl webFingerInstance = _accountBuilder.webFingerSelectedInstance();
        if (!webFingerInstance.isEmpty()) {
            return webFingerInstance;
        } else {
            return _accountBuilder.serverUrl();
        }
    }();

    return _accountBuilder.authenticationStrategy()->makeFetchUserInfoJobFactory(_accessManager).startJob(serverUrl, parent);
}

} // APP::Wizard
