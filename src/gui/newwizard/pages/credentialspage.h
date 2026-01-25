#pragma once

#include "device/devicetypes.h"
#include "setuppagetype.h"
#include "pagecontext.h"

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

    Q_PROPERTY(bool progressVisible READ progressVisible WRITE setProgressVisible NOTIFY progressVisibleChanged FINAL)

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

    void showErrorMessage(const QString& msg, const QString& tooltip = QStringLiteral(""));
    void showInvalidUrlError();
    void showInvalidCredentialsError();
    void showProgressIndicator(bool show);

    void showDevicesInfo(bool show);

    void setProgressVisible(bool visible);
    bool progressVisible() const {return progressVisible_;}

Q_SIGNALS:
    void actionTriggered(CredentialsAction action, std::optional<CredentialsContext> ctx = std::nullopt);
    void progressVisibleChanged();

private:
    void onTextEdited(const QString& txt);

    void validateFormData();
    bool isAllFieldNotEmpty();

private:
    Ui::CredentialsPage* ui = nullptr;

    QList<Device> dev_list;
    bool progressVisible_ = false;
};
