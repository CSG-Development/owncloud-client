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

#include "credentialsstate.h"
#include "jobs/checkbasicauthjobfactory.h"
#include "networkjobs/fetchuserinfojobfactory.h"

namespace APP::Wizard {

CredentialsState::CredentialsState(SetupWizardContext *context)
    : AbstractState(context)
{
    // if (!context->accountBuilder().legacyWebFingerUsername().isEmpty()) {
    //     _page = CredentialsPage::createForWebFinger(_context->accountBuilder().serverUrl(), _context->accountBuilder().legacyWebFingerUsername());
    // } else {
    //     _page = new CredentialsPage(_context->accountBuilder().serverUrl());
    // }
    _page = new CredentialsPage;
}

void CredentialsState::evaluatePage()
{
    auto *credentialsPage = qobject_cast<CredentialsPage *>(_page);
    Q_ASSERT(credentialsPage != nullptr);

    const QString username = credentialsPage->username();
    const QString password = credentialsPage->password();

    _context->accountBuilder().setAuthenticationStrategy(new HttpBasicAuthenticationStrategy(username, password));

    if (!_context->accountBuilder().hasValidCredentials()) {
        Q_EMIT evaluationFailed(tr("Invalid credentials"));
    }
    Q_EMIT evaluationSuccessful();
}

SetupWizardState CredentialsState::state() const
{
    return SetupWizardState::CredentialsState;
}

} // APP::Wizard
