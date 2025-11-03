#include "credentialspage.h"
#include "ui_credentialspage.h"

#include "gui/customui/stylehelper.h"
#include "gui/customui/focusproxy.h"
#include "gui/customui/dimwidget.h"
#include "codedialog.h"
#include "theme.h"

#include <QLineEdit>
#include <QComboBox>
#include <QRegularExpression>
#include <QHostAddress>

namespace {
constexpr int fontSize = 16;
QPair<QString,QString> widgetStyle = {
    QStringLiteral(":/res/login/cred_page_light.qss"),
    QStringLiteral(":/res/login/cred_page_dark.qss")
};
QPair<QString,QString> refreshIcon = {
    QStringLiteral(":/res/login/refresh_light.svg"),
    QStringLiteral(":/res/login/refresh_dark.svg")
};
QPair<QString,QString> backIcon = {
    QStringLiteral(":/res/login/arrow_back_btn_light.svg"),
    QStringLiteral(":/res/login/arrow_back_btn_dark.svg")
};
constexpr int code_expire_seconds = 10 * 60;     // 10 min
}

CredentialsPage::CredentialsPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CredentialsPage)
{
    ui->setupUi(this);
    setObjectName("credentialsPage");
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);

    auto noFocus = new FocusProxyStyle;
    ui->btnLogin->setStyle(noFocus);
    ui->btnCancel->setStyle(noFocus);
    ui->btnResetPass->setStyle(noFocus);

    setAttribute(Qt::WA_TranslucentBackground, true);

    dim = new DimWidget(this);
    dim->setVisible(false);

    codeDialog = new CodeDialog(this);
    codeDialog->setVisible(false);

    connect(codeDialog, &CodeDialog::skipClicked, this, [&] {
        codeDialog->setVisible(false);
        codeDialog->clearCode();
        dim->setVisible(false);
        emit codeSkipped();
    });
    connect(codeDialog, &CodeDialog::allowAccessClicked, this, [&] {
        codeDialog->setVisible(false);
        codeDialog->clearCode();
        dim->setVisible(false);
        emit codeEntered(codeDialog->getCode());
    });
    connect(codeDialog, &CodeDialog::resendCodeClicked, this, [&] {
        codeExpireTime = QDateTime::currentDateTime().addSecs(code_expire_seconds);
        codeExpireCheckTimer.start();
        emit codeResend();
    });

    connect(ui->btnLogin, &QPushButton::clicked, this, [&] {
        Q_EMIT loginClicked(url(), email(), password());
    });
    connect(ui->btnCancel, &QPushButton::clicked, this, &CredentialsPage::cancelClicked);
    connect(ui->btnSettings, &QToolButton::clicked, this, &CredentialsPage::settingsClicked);
    connect(ui->btnResetPass, &QPushButton::clicked, this, &CredentialsPage::resetPasswordClicked);
    connect(ui->btnRefresh, &QToolButton::clicked, this, [&] {
        showErrorMessage({});
        showProgressIndicator(true);
        emit refreshDevicesClicked();
    });
    connect(ui->btnBack, &QToolButton::clicked, this, &CredentialsPage::backButtonClicked);

    // MacOS hover enable
    ui->btnLogin->setAttribute(Qt::WA_Hover, true);
    ui->btnCancel->setAttribute(Qt::WA_Hover, true);
    ui->btnSettings->setAttribute(Qt::WA_Hover, true);
    ui->btnRefresh->setAttribute(Qt::WA_Hover, true);
    ui->btnBack->setAttribute(Qt::WA_Hover, true);

    ui->btnLogin->setMouseTracking(true);

    ui->edUrl->setPlaceholderText(tr("Connecting to"));
    ui->edUrl->setFontPixelSize(fontSize);

    ui->edPassword->setPlaceholderText(tr("Password"));
    ui->edPassword->setFontPixelSize(fontSize);
    ui->edPassword->setPasswordMode(true);

    connect(ui->edUrl, &ComboWidget::textEdited, this, &CredentialsPage::onTextEdited);
    connect(ui->edPassword, &InputWidget::textEdited, this, &CredentialsPage::onTextEdited);

    validateFormData();

    ui->btnLogin->installEventFilter(this);
    ui->btnLogin->setToolTip(tr("Enter a valid email address and password"));

    showErrorMessage({});
    showProgressIndicator(false);

    codeExpireCheckTimer.setInterval(1000);
    connect(&codeExpireCheckTimer, &QTimer::timeout, this, &CredentialsPage::onCodeExpireCheckTimer);

    updateTheme();
}

CredentialsPage::~CredentialsPage()
{
    delete ui;
}

bool CredentialsPage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->btnLogin) {
        if (event->type() == QEvent::EnabledChange) {
            if (ui->btnLogin->isEnabled()) {
                ui->btnLogin->setToolTip({});
            }
            else {
                ui->btnLogin->setToolTip(tr("Enter a valid email address and password"));
            }

        }
    }
    return QWidget::eventFilter(watched, event);
}

void CredentialsPage::updateTheme()
{
    bool isDark = CUR::Theme::instance()->isDarkTheme();

    const QList<QWidget*> childrenList = findChildren<QWidget*>();
    for (auto* widget: childrenList) {
        if (widget->metaObject()->indexOfSlot("setDarkTheme()") != -1) {
            QMetaObject::invokeMethod(widget, "setDarkTheme");
        }
    }

    ui->btnRefresh->setIcon(isDark ? QIcon(refreshIcon.second) : QIcon(refreshIcon.first));
    ui->btnBack->setIcon(isDark ? QIcon(backIcon.second) : QIcon(backIcon.first));
    setStyleSheet(CUR::StyleHelper::loadFileToString(isDark ? widgetStyle.second : widgetStyle.first));

    update();
}

void CredentialsPage::setDevicesList(const QList<Device> &list)
{
    ui->edUrl->setItems(list);
}

std::optional<Device> CredentialsPage::currentDevice() const
{
    return ui->edUrl->currentDevice();
}

QString CredentialsPage::url() const
{
    if (auto currDevice = currentDevice()) {
        if (!currDevice->paths.isEmpty()) {
            auto path = Device::firstRemotePath(currDevice.value());
            DevicePath devPath;
            if (path) {
                devPath = path.value();
            }
            else {
                devPath = currDevice->paths.first();
            }

            return normalizeUrl(devPath.address, devPath.port, true);
        }
    }
    return {};
}

QString CredentialsPage::email() const
{
    return ui->lblEmail->text();
}

void CredentialsPage::setEmail(const QString &user)
{
    ui->lblEmail->setText(user);
}

QString CredentialsPage::password() const
{
    return ui->edPassword->text();
}

void CredentialsPage::showErrorMessage(const QString& msg)
{
    ui->frameErrorMessage->setVisible(!msg.isEmpty());
    ui->lblErrorText->setText(msg);
}

void CredentialsPage::showInvalidUrlError()
{
    ui->edUrl->setErrorState(true, tr("Invalid URL"));
    ui->btnLogin->setEnabled(false);
}

void CredentialsPage::showInvalidCredentialsError()
{
    ui->edPassword->setErrorState(true, {});
    ui->btnLogin->setEnabled(false);
}

void CredentialsPage::showProgressIndicator(bool show)
{
    ui->btnRefresh->setVisible(!show);
    ui->progressIndicator->setVisible(show);
}

void CredentialsPage::showCodeDialog()
{
    showProgressIndicator(false);
    codeExpireTime = QDateTime::currentDateTime().addSecs(code_expire_seconds);
    codeExpireCheckTimer.start();

    dim->setVisible(true);

    codeDialog->show();
}

void CredentialsPage::onTextEdited(const QString&/*txt*/)
{
    if (sender() == ui->edUrl) {
        ui->edUrl->setErrorState(false);
    }
    else if (sender() == ui->edPassword) {
        ui->edPassword->setErrorState(false);
    }

    validateFormData();
    showErrorMessage({});
}

void CredentialsPage::onCodeExpireCheckTimer()
{
    if (QDateTime::currentDateTime() > codeExpireTime) {
        codeExpireCheckTimer.stop();
        codeDialog->showError(tr("Your code has expired"));
    }
}

void CredentialsPage::validateFormData()
{
    bool valid = isAllFieldNotEmpty();
    ui->btnLogin->setEnabled(valid);
}

bool CredentialsPage::isAllFieldNotEmpty()
{
    return !ui->edUrl->text().trimmed().isEmpty() &&
           !ui->edPassword->text().trimmed().isEmpty();
}
