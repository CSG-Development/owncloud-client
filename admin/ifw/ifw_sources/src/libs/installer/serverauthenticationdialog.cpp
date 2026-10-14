/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#include "serverauthenticationdialog.h"

#include "downloadfiletask.h"
#include "ui_serverauthenticationdialog.h"

#include <QAuthenticator>

namespace QInstaller {

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::ServerAuthenticationDialog
    \internal
*/

ServerAuthenticationDialog::ServerAuthenticationDialog(const QString &m, const FileTaskItem &item)
    : ui(new Ui::ServerAuthenticationDialog)
{
    ui->setupUi(this);
    ui->siteDescription->setText(m);
    setWindowFlags(windowFlags() &~Qt::WindowContextHelpButtonHint);

    const QAuthenticator auth = item.value(TaskRole::Authenticator).value<QAuthenticator>();
    ui->userEdit->setText(auth.user());
    ui->passwordEdit->setText(auth.password());
}

ServerAuthenticationDialog::~ServerAuthenticationDialog()
{
    delete ui;
}

QString ServerAuthenticationDialog::user() const
{
    return ui->userEdit->text();
}

QString ServerAuthenticationDialog::password() const
{
    return ui->passwordEdit->text();
}

} // namespace QInstaller
