#pragma once

#include "enums.h"
#include "gui/settingsdialog.h"
#include "loginservices/devicetypes.h"

namespace Ui {class SetupWidget;}

class CredentialsPage;
class WaitPage;
class FinishedPage;
class EmailPage;

namespace CUR::Wizard {

class SetupContext;

class SetupWidget : public QWidget
{
    Q_OBJECT

public:
    enum class SetupPage {
        PageEmail = 0,
        PageCredentials,
        PageWait,
        PageFinished
    };

    explicit SetupWidget(SettingsDialog *parent);
    ~SetupWidget() noexcept override;

    void displayPage(SetupPage page);

    void showErrorMessage(const QString &errorMessage);
    void hideErrorMessage();

    void showCodeDialog();
    void setDevicesList(const QList<Device> &list);
    void setEmail(const QString& email);

    void onCancelClicked();
    void onSetupFinishPageDefaults(const QString &defaultSyncTargetDir, const QString &userChosenSyncTargetDir,
        bool vfsIsAvailable, bool enableVfsByDefault, bool vfsModeIsExperimental);

    void setInvalidUrlError();
    void setInvalidCredentialsError();

    void showCredPageProgress(bool show);

Q_SIGNALS:
    void rejected();

    void loginEmailClicked(const QString& user);
    void loginCredentialClicked(const QString& url, const QString& user, const QString& password);
    void loginSettingsClicked();
    void loginResetPasswordClicked();
    void refreshDevicesClicked();
    void finishPageBackClicked();
    void finishPageDoneClicked(CUR::Wizard::SyncMode mode, const QString& targetDir);
    void codeEntered(const QString& code);
    void codeSkipped();
    void credPageBackClicked();


private:
    void onThemeChanged();

    ::Ui::SetupWidget *_ui = nullptr;

    EmailPage* emailPage_ = nullptr;
    CredentialsPage* credPage_ = nullptr;
    WaitPage* waitPage_ = nullptr;
    FinishedPage* finishPage_ = nullptr;
};
}
