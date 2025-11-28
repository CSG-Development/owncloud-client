#pragma once

#include "enums.h"
#include "gui/settingsdialog.h"
#include "devicetypes.h"

namespace Ui {class SetupWidget;}

class CredentialsPage;
class WaitPage;
class FinishedPage;
class EmailPage;
class ConnectErrorPage;

namespace CUR {enum class RemoteRequest;}

namespace CUR::Wizard {

class SetupContext;

class SetupWidget : public QWidget
{
    Q_OBJECT

public:
    enum class SetupPage {
        PageNone = 0,
        PageEmail,
        PageCredentials,
        PageWait,
        PageFinished,
        PageConnectError
    };

    explicit SetupWidget(SettingsDialog *parent);
    ~SetupWidget() noexcept override;

    void displayPage(SetupPage page);
    void displayPreviousPage();

    void showErrorMessage(const QString &errorMessage);
    void hideErrorMessage();

    void codeRequested();
    void codeAccepted();

    void setDevicesList(const QList<Device> &list);
    void setEmail(const QString& email);

    void onCancelClicked();
    void onSetupFinishPageDefaults(const QString &defaultSyncTargetDir, const QString &userChosenSyncTargetDir,
        bool vfsIsAvailable, bool enableVfsByDefault, bool vfsModeIsExperimental);

    void errorOccured(RemoteRequest req, int code, const QString &message);

    void setInvalidUrlError();
    void setInvalidCredentialsError();

    void showCredPageProgress(bool show);

Q_SIGNALS:
    void rejected();

    void loginEmailClicked(const QString& user);
    void loginCredentialClicked(const Device& dev, const QString& user, const QString& password);
    void loginSettingsClicked();
    void loginResetPasswordClicked();
    void refreshDevicesClicked();
    void finishPageBackClicked();
    void finishPageDoneClicked(CUR::Wizard::SyncMode mode, const QString& targetDir);
    void codeEntered(const QString& code);
    void codeSkipped();
    void codeResendClicked();
    void credPageBackClicked();
    void connectErrorPageBackClicked();
    void connectErrorPageRetryClicked();

private:
    void onThemeChanged();

    ::Ui::SetupWidget *_ui = nullptr;

    EmailPage* emailPage_ = nullptr;
    CredentialsPage* credPage_ = nullptr;
    WaitPage* waitPage_ = nullptr;
    FinishedPage* finishPage_ = nullptr;
    ConnectErrorPage* connectErrorPage_ = nullptr;

    SetupPage currentPage = SetupPage::PageNone;
    SetupPage previousPage = SetupPage::PageNone;
};
}
