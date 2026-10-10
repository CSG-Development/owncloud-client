/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#ifndef SERVERAUTHENTICATIONDIALOG_H
#define SERVERAUTHENTICATIONDIALOG_H

#include <QDialog>

namespace QInstaller {

namespace Ui {
    class ServerAuthenticationDialog;
}
class FileTaskItem;

class ServerAuthenticationDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(ServerAuthenticationDialog)

public:
    explicit ServerAuthenticationDialog(const QString &message, const FileTaskItem &item);
    ~ServerAuthenticationDialog();

    QString user() const;
    QString password() const;

private:
    Ui::ServerAuthenticationDialog *ui;
};

} // QInstaller

#endif // SERVERAUTHENTICATIONDIALOG_H
