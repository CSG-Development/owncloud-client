#pragma once

#include "loginservices/devicetypes.h"

#include <QWidget>
#include <QTimer>
#include <QDateTime>

namespace Ui {class CredentialsPage;}

class DimWidget;
class CodeDialog;

class CredentialsPage : public QWidget
{
    Q_OBJECT

public:
    explicit CredentialsPage(QWidget *parent = nullptr);
    ~CredentialsPage();

    bool eventFilter(QObject *watched, QEvent *event) override;

    void updateTheme();

    void setDevicesList(const QList<DeviceInfo>& list);
    std::optional<DeviceInfo> currentDevice() const;

    QString url() const;
    // Adds 'https://' prefix and '/files' suffix if missed
    // Adds port if specified
    static QString normalizeUrl(const QString& url, int port = 0);

    QString email() const;
    void setEmail(const QString& user);

    QString password() const;

    void showErrorMessage(const QString& msg);
    void showInvalidUrlError();
    void showInvalidCredentialsError();
    void showProgressIndicator(bool show);
    void showCodeDialog();

Q_SIGNALS:
    void loginClicked(const QString& url, const QString& user, const QString& password);
    void cancelClicked();
    void settingsClicked();
    void resetPasswordClicked();
    void refreshDevicesClicked();
    void backButtonClicked();

    void codeEntered(const QString& code);
    void codeSkipped();
    void codeResend();

private:
    void onTextEdited(const QString& txt);
    void onCodeExpireCheckTimer();

    void validateFormData();
    bool isAllFieldNotEmpty();

private:
    Ui::CredentialsPage* ui = nullptr;

    DimWidget* dim = nullptr;
    QTimer codeExpireCheckTimer;
    QDateTime codeExpireTime;
    CodeDialog* codeDialog = nullptr;
};
