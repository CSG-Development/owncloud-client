#pragma once

#include "loginservices/devicetypes.h"

#include <QWidget>

namespace Ui {class CredentialsPage;}

class CredentialsPage : public QWidget
{
    Q_OBJECT

public:
    explicit CredentialsPage(QWidget *parent = nullptr);
    ~CredentialsPage();

    bool eventFilter(QObject *watched, QEvent *event) override;

    void updateTheme();

    void setDevicesList(const QList<DeviceInfo>& list);

    QString url() const;

    QString email() const;
    void setEmail(const QString& user);

    QString password() const;

    void showErrorMessage(const QString& msg);
    void showInvalidUrlError();
    void showInvalidCredentialsError();

Q_SIGNALS:
    void loginClicked(const QString& url, const QString& user, const QString& password);
    void cancelClicked();
    void settingsClicked();
    void resetPasswordClicked();
    void refreshDevicesClicked();

private:
    void onTextEdited(const QString& txt);

    void validateFormData();

    // UrlValidate return true if input string is empty to avoid startup errors
    bool simpleUrlValidate(const QString& url);

    bool isAllFieldNotEmpty();

    Ui::CredentialsPage* ui = nullptr;
};
