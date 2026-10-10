/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#ifndef PROXYCREDENTIALSDIALOG_H
#define PROXYCREDENTIALSDIALOG_H

#include <QDialog>

QT_FORWARD_DECLARE_CLASS(QNetworkProxy)

namespace QInstaller {

namespace Ui {
class ProxyCredentialsDialog;
}

class ProxyCredentialsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProxyCredentialsDialog(const QNetworkProxy &proxy, QWidget *parent = 0);
    ~ProxyCredentialsDialog();

    QString userName() const;
    void setUserName(const QString &username);
    QString password() const;
    void setPassword(const QString &passwd);

private:
    Ui::ProxyCredentialsDialog *ui;
};

} // QInstaller

#endif // PROXYCREDENTIALSDIALOG_H
