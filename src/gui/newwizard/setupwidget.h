#pragma once

#include "enums.h"
#include "pages/setuppagetype.h"
#include "gui/settingsdialog.h"
#include "device/devicetypes.h"
#include "pages/pagecontext.h"

namespace Ui {class SetupWidget;}

class CredentialsPage;
class WaitPage;
class FinishedPage;
class EmailPage;
class ConnectErrorPage;

namespace APP {enum class RemoteRequest;}

namespace APP::Wizard {

class SetupContext;

class SetupWidget : public QWidget
{
    Q_OBJECT

public:
    struct GuiContext {
        QString errorString;
        QString email;
        bool showBusy = false;
        bool showError = false;
    };

    explicit SetupWidget(SettingsDialog *parent);
    ~SetupWidget() noexcept override;

    void displayPage(SetupPage page, std::optional<GuiContext> ctx = std::nullopt);
    void displayPreviousPage();

    void showErrorMessage(const QString &errorMessage);
    void hideErrorMessage();

    void setEmailIsNotAllowed(bool val);

    void setDevicesList(const DeviceList& list);
    void setEmail(const QString& email);

    void onCancelClicked();
    void onSetupFinishPageDefaults(const QString &defaultSyncTargetDir, const QString &userChosenSyncTargetDir,
                                   bool vfsIsAvailable, bool enableVfsByDefault, bool vfsModeIsExperimental);

    void setInvalidUrlError();
    void setInvalidCredentialsError();
    void setCredErrorMessage(const QString& error, const QString& tooltip = QStringLiteral(""));

    void showCredPageProgress(bool show);

    SetupPage currentPage() const {return currentPage_;}

Q_SIGNALS:
    void rejected();

    void credentialsAction(CredentialsAction action, std::optional<CredentialsContext> ctx = std::nullopt);

    // void startDeviceListManager(const QString& email);
    void loginEmailClicked(const QString& user);
    void finishPageBackClicked();
    void finishPageDoneClicked(APP::Wizard::SyncMode mode, const QString& targetDir);
    void connectErrorPageBackClicked();
    void connectErrorPageRetryClicked();

private:
    void onCredentialsAction(CredentialsAction action, std::optional<CredentialsContext> ctx = std::nullopt);

    void transitionTo(SetupPage newPage);
    void processPageChange();

    void onThemeChanged(bool isDark);
    void setSafeCurrentWidget(QWidget* w);

    ::Ui::SetupWidget *_ui = nullptr;

    EmailPage* emailPage_ = nullptr;
    CredentialsPage* credPage_ = nullptr;
    WaitPage* waitPage_ = nullptr;
    FinishedPage* finishPage_ = nullptr;
    ConnectErrorPage* connectErrorPage_ = nullptr;

    SetupPage currentPage_ = SetupPage::PageNone;
    SetupPage previousPage = SetupPage::PageNone;
    std::optional<GuiContext> guiContext;
};
}
