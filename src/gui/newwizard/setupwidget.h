#pragma once

#include "enums.h"
#include "gui/settingsdialog.h"

namespace Ui {class SetupWidget;}

class CredentialsPage;
class WaitPage;
class FinishedPage;

namespace CUR::Wizard {

class SetupContext;

class SetupWidget : public QWidget
{
    Q_OBJECT

public:
    enum class SetupPage {
        PageCredentials = 0,
        PageWait,
        PageFinished
    };

    explicit SetupWidget(SettingsDialog *parent);
    ~SetupWidget() noexcept override;

    void displayPage(SetupPage page);

    void showErrorMessage(const QString &errorMessage);
    void hideErrorMessage();

    void onCancelClicked();
    void onSetupFinishPageDefaults(const QString &defaultSyncTargetDir, const QString &userChosenSyncTargetDir,
        bool vfsIsAvailable, bool enableVfsByDefault, bool vfsModeIsExperimental);

Q_SIGNALS:
    void rejected();

    void loginClicked(const QString& url, const QString& user, const QString& password);
    void loginSettingsClicked();
    void loginResetPasswordClicked();
    void finishPageBackClicked();
    void finishPageDoneClicked(CUR::Wizard::SyncMode mode, const QString& targetDir);

private:
    void onThemeChanged();

    ::Ui::SetupWidget *_ui;

    CredentialsPage* credPage_ = nullptr;
    WaitPage* waitPage_ = nullptr;
    FinishedPage* finishPage_ = nullptr;
};
}
