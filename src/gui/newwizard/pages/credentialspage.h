#pragma once

#include "device/devicetypes.h"
#include "setuppagetype.h"
#include "pagecontext.h"

#include <QWidget>
#include <QTimer>
#include <QDateTime>
#include <QProperty>

namespace Ui {class CredentialsPage;}

class CredentialsPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool progressVisible READ progressVisible WRITE setProgressVisible NOTIFY progressVisibleChanged FINAL)
    Q_PROPERTY(bool darkTheme READ isDarkTheme WRITE setDarkTheme BINDABLE bindableDarkTheme)

public:
    explicit CredentialsPage(QWidget *parent = nullptr);
    ~CredentialsPage();

    bool eventFilter(QObject *watched, QEvent *event) override;

    void updateTheme();

    void setDevicesList(const DeviceList& list);
    std::optional<Device> currentDevice() const;

    QString email() const;
    void setEmail(const QString& user);

    QString password() const;

    void showErrorMessage(const QString& msg, const QString& tooltip = QStringLiteral(""));
    void showInvalidUrlError();
    void showInvalidCredentialsError();
    void showProgressIndicator(bool show);

    void showDevicesInfo(bool show);

    void setProgressVisible(bool visible);
    bool progressVisible() const {return progressVisible_;}

    bool isDarkTheme() const { return darkTheme_.value(); }
    void setDarkTheme(bool v) { darkTheme_.setValue(v); }

    QBindable<bool> bindableDarkTheme() {return &darkTheme_;}

Q_SIGNALS:
    void actionTriggered(CredentialsAction action, std::optional<CredentialsContext> ctx = std::nullopt);
    void progressVisibleChanged();

private:
    void onTextEdited(const QString& txt);

    void validateFormData();
    bool isAllFieldNotEmpty();
    void loadFavDevice();

private:
    Ui::CredentialsPage* ui = nullptr;

    DeviceList dev_list;
    bool progressVisible_ = false;
    QPropertyNotifier themeNotifier;
    QProperty<bool> darkTheme_ {false};
};
