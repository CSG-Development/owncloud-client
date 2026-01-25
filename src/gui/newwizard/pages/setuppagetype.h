#pragma once

#include <QString>
#include <QMap>

enum class SetupPage {
    PageNone = 0,
    PageEmail,
    PageCredentials,
    PageWait,
    PageFinished,
    PageConnectError
};

enum class CredentialsAction {
    CancelClicked,
    LoginClicked,
    SettingsClicked,
    ResetPasswordClicked,
    RefreshDevicesClicked,
    BackButtonClicked,
    CantFindDeviceClicked
};

inline QString CredentialsActionToStr(CredentialsAction act) {
    QMap<CredentialsAction,QString> map = {
        {CredentialsAction::CancelClicked, QStringLiteral("CancelClicked")},
        {CredentialsAction::LoginClicked, QStringLiteral("LoginClicked")},
        {CredentialsAction::SettingsClicked, QStringLiteral("SettingsClicked")},
        {CredentialsAction::ResetPasswordClicked, QStringLiteral("ResetPasswordClicked")},
        {CredentialsAction::RefreshDevicesClicked, QStringLiteral("RefreshDevicesClicked")},
        {CredentialsAction::BackButtonClicked, QStringLiteral("BackButtonClicked")},
        {CredentialsAction::CantFindDeviceClicked, QStringLiteral("CantFindDeviceClicked")},
    };
    if (map.contains(act))
        return map[act];
    return {};
}
