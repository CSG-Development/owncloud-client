#pragma once

#include "devicetypes.h"

#include <QWidget>
#include <QTimer>
#include <QDateTime>

namespace Ui {class CredentialsPage;}
namespace CUR {enum class RemoteRequest;}

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

    void setDevicesList(const QList<Device>& list);
    std::optional<Device> currentDevice() const;

    QString email() const;
    void setEmail(const QString& user);

    QString password() const;

    void showErrorMessage(const QString& msg);
    void showInvalidUrlError();
    void showInvalidCredentialsError();
    void showProgressIndicator(bool show);

    void showCodeDialog(bool show);
    bool isCodeDialogVisible() const;
    void showCodeInvalidError();
    void showCodeExpiredError();
    void showCodeServerError();
    void errorOccured(CUR::RemoteRequest req, int code, const QString& message);

    void codeJustRequested();

    void showDevicesInfo(bool show);

Q_SIGNALS:
    void loginClicked(const Device& dev, const QString& user, const QString& password);
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
    bool isCodeExpired = false;
    QList<Device> dev_list;
};
