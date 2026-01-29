#include "credentialspage.h"
#include "ui_credentialspage.h"

#include "gui/customui/stylehelper.h"
#include "gui/customui/focusproxy.h"
#include "gui/customui/dimwidget.h"
#include "theme.h"

#include <QLineEdit>
#include <QComboBox>
#include <QRegularExpression>
#include <QHostAddress>
#include <QToolTip>
#include <QLoggingCategory>

namespace {
constexpr int fontSize = 16;
std::pair<QString,QString> refreshIcon = {
    QStringLiteral(":/res/login/refresh_light.svg"),
    QStringLiteral(":/res/login/refresh_dark.svg")
};
std::pair<QString,QString> backIcon = {
    QStringLiteral(":/res/login/arrow_back_btn_light.svg"),
    QStringLiteral(":/res/login/arrow_back_btn_dark.svg")
};
constexpr int code_expire_seconds = 10 * 60;     // 10 min
}

Q_LOGGING_CATEGORY(lcCredPage, "gui.page.credential", QtDebugMsg)

CredentialsPage::CredentialsPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CredentialsPage)
{
    ui->setupUi(this);

    setStyleSheet(CUR::StyleHelper::loadFileToString(QStringLiteral(":/res/login/cred_page.qss")));

    themeNotifier = darkTheme_.addNotifier([this] {
        updateTheme();
    });
    darkTheme_.setValue(CUR::Theme::instance()->isDarkTheme());

    setObjectName("credentialsPage");
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);

    ui->btnLogin->setStyle(new FocusProxyStyle(ui->btnLogin));
    ui->btnCancel->setStyle(new FocusProxyStyle(ui->btnCancel));
    ui->btnResetPass->setStyle(new FocusProxyStyle(ui->btnResetPass));
    ui->btnCantFindDevice->setStyle(new FocusProxyStyle(ui->btnCantFindDevice));

    setAttribute(Qt::WA_TranslucentBackground, true);

    connect(ui->btnLogin, &QPushButton::clicked, this, [this] {
        CredentialsContext ctx{currentDevice(), email(), password()};
        emit actionTriggered(CredentialsAction::LoginClicked, ctx);
    });

    connect(ui->btnCancel, &QPushButton::clicked, this, [this] {
        emit actionTriggered(CredentialsAction::CancelClicked);
    });
    connect(ui->btnSettings, &QToolButton::clicked, this, [this] {
        showDevicesInfo(true);
        emit actionTriggered(CredentialsAction::SettingsClicked);
    });

    connect(ui->btnCantFindDevice, &QPushButton::clicked, this, [this] {
        emit actionTriggered(CredentialsAction::CantFindDeviceClicked);
    });
    connect(ui->btnResetPass, &QPushButton::clicked, this, [this] {
        emit actionTriggered(CredentialsAction::ResetPasswordClicked);
    });

    connect(ui->btnRefresh, &QToolButton::clicked, this, [this] {
        showErrorMessage({});
        showProgressIndicator(true);
        emit actionTriggered(CredentialsAction::RefreshDevicesClicked);
    });
    connect(ui->btnBack, &QToolButton::clicked, this, [this] {
        emit actionTriggered(CredentialsAction::BackButtonClicked);
    });

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

    ui->btnSettings->setVisible(false);

    showErrorMessage({});
    showProgressIndicator(false);

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
    ui->btnRefresh->setIcon(darkTheme_.value() ? QIcon(refreshIcon.second) : QIcon(refreshIcon.first));
    ui->btnBack->setIcon(darkTheme_.value() ? QIcon(backIcon.second) : QIcon(backIcon.first));
    CUR::StyleHelper::setTheme(this, darkTheme_.value());

    update();
}

void CredentialsPage::setDevicesList(const QList<Device> &list)
{
    dev_list = list;
    // DEBUG
    // for (const auto& d: dev_list)
    //     qCDebug(lcCredPage) << d.toString();
    ui->edUrl->setItems(list);
}

std::optional<Device> CredentialsPage::currentDevice() const
{
    return ui->edUrl->currentDevice();
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

void CredentialsPage::showErrorMessage(const QString& msg, const QString& tooltip)
{
    showProgressIndicator(false);
    ui->frameErrorMessage->setVisible(!msg.isEmpty());
    ui->lblErrorText->setText(msg);
    ui->lblErrorText->setToolTip(tooltip);
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
    ui->edPassword->setEnabled(!show);
    ui->btnCantFindDevice->setEnabled(!show);
    ui->btnResetPass->setEnabled(!show);
    ui->btnBack->setEnabled(!show);
    ui->edUrl->setEnabled(!show);

    ui->btnRefresh->setVisible(!show);
    ui->progressIndicator->setIndicatorVisible(show);
    update();
    qApp->processEvents();
}

void CredentialsPage::showDevicesInfo(bool show)
{
    QString s;
    for (const auto& d: std::as_const(dev_list)) {
        s += QStringLiteral("<b>%1</b><br>").arg(d.certificateCommonName);
        s += QStringLiteral("  Friendly: %1<br>").arg(d.friendlyName);
        s += QStringLiteral("  CN: %1<br>").arg(d.certificateCommonName);
        if (d.paths.isEmpty()) {
            s += QStringLiteral("  no paths defined<br>");
        }
        else {
            for (const auto& p: d.paths) {
                s += QStringLiteral("  %1 %2 type: %3 origin: %4<br>")
                         .arg(p.address)
                         .arg(p.port == 0 ? QStringLiteral("") : QStringLiteral("port: %1").arg(p.port))
                         .arg(DevHelpers::devTypeToStr(p.deviceType))
                         .arg(DevHelpers::originToStr(p.origin));
            }
        }
    }
    auto r = mapToGlobal(geometry().topLeft());
    if (show)
        QToolTip::showText(r, s);
    else
        QToolTip::showText(r, QStringLiteral(""));
}

void CredentialsPage::setProgressVisible(bool visible)
{
    progressVisible_ = visible;
    emit progressVisibleChanged();
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
