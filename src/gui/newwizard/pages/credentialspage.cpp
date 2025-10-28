#include "credentialspage.h"
#include "ui_credentialspage.h"

#include "gui/customui/stylehelper.h"
#include "gui/customui/focusproxy.h"
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
    QStringLiteral(":/res/login/refresh_light.svg")
};
}

CredentialsPage::CredentialsPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CredentialsPage)
{
    ui->setupUi(this);
    setObjectName("credentialsPage");

    auto noFocus = new FocusProxyStyle;
    ui->btnLogin->setStyle(noFocus);
    ui->btnCancel->setStyle(noFocus);
    ui->btnResetPass->setStyle(noFocus);

    setAttribute(Qt::WA_TranslucentBackground, true);

    connect(ui->btnLogin, &QPushButton::clicked, this, [&] {
        Q_EMIT loginClicked(url(), email(), password());
    });
    connect(ui->btnCancel, &QPushButton::clicked, this, &CredentialsPage::cancelClicked);
    connect(ui->btnSettings, &QToolButton::clicked, this, &CredentialsPage::settingsClicked);
    connect(ui->btnResetPass, &QPushButton::clicked, this, &CredentialsPage::resetPasswordClicked);
    connect(ui->btnRefresh, &QToolButton::clicked, this, &CredentialsPage::refreshDevicesClicked);

    ui->btnLogin->setMouseTracking(true);

    ui->edEmail->setPlaceholderText(tr("Email"));
    ui->edEmail->setFontPixelSize(fontSize);

    ui->edUrl->setPlaceholderText(tr("Connecting to"));
    ui->edUrl->setFontPixelSize(fontSize);

    ui->edPassword->setPlaceholderText(tr("Password"));
    ui->edPassword->setFontPixelSize(fontSize);
    ui->edPassword->setPasswordMode(true);

    connect(ui->edUrl, &ComboWidget::textEdited, this, &CredentialsPage::onTextEdited);
    connect(ui->edEmail, &InputWidget::textEdited, this, &CredentialsPage::onTextEdited);
    connect(ui->edPassword, &InputWidget::textEdited, this, &CredentialsPage::onTextEdited);

    connect(ui->edUrl, &ComboWidget::focusLost, this, [&] {
        if (!simpleUrlValidate(url())) {
            ui->edUrl->setErrorState(true, tr("Invalid URL"));
            ui->btnLogin->setEnabled(false);
        }
    });

    connect(ui->edEmail, &InputWidget::focusReceived, this, [&] {
        ui->edEmail->setErrorState(false);
    });
    ui->edEmail->setReadOnly(true);

    validateFormData();

    ui->btnLogin->installEventFilter(this);
    ui->btnLogin->setToolTip(tr("Enter a valid email address and password"));

    showErrorMessage({});

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
    setStyleSheet(CUR::StyleHelper::loadFileToString(isDark ? widgetStyle.second : widgetStyle.first));

    update();
}

void CredentialsPage::setDevicesList(const QList<DeviceInfo> &list)
{
    auto cb = ui->edUrl->comboBox();
    cb->clear();

    for (const auto& item: list) {
        if (item.port > 0) {
            QUrl addr(item.host);
            addr.setPort(item.port);
            cb->addItem(addr.toString());
        }
        else {
            cb->addItem(item.host);
        }
    }
}

QString CredentialsPage::url() const
{
    return ui->edUrl->text();
}

QString CredentialsPage::email() const
{
    return ui->edEmail->text();
}

void CredentialsPage::setEmail(const QString &user)
{
    const QSignalBlocker b_(ui->edEmail);
    ui->edEmail->setText(user);
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
    ui->edEmail->setErrorState(true, {});
    ui->edPassword->setErrorState(true, {});
    ui->btnLogin->setEnabled(false);
}

void CredentialsPage::onTextEdited(const QString&/*txt*/)
{
    if (sender() == ui->edEmail) {
        ui->edEmail->setErrorState(false);
    }
    else if (sender() == ui->edUrl) {
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
    bool valid = isAllFieldNotEmpty() && simpleUrlValidate(url());

    ui->btnLogin->setEnabled(valid);
}

bool CredentialsPage::simpleUrlValidate(const QString& url)
{
    if (url.trimmed().isEmpty())
        return true;

    QString urlRegExp = QStringLiteral(
                        "^"
                        // protocol identifier
                        "(?:(?:https?|ftp)://)"
                        // user:pass authentication
                        "(?:\\S+(?::\\S*)?&#64;)?"
                        "(?:"
                        // IP address exclusion
                        // private & local networks
                        "(?!(?:10|127)(?:\\.\\d{1,3}){3})"
                        "(?!(?:169\\.254|192\\.168)(?:\\.\\d{1,3}){2})"
                        "(?!172\\.(?:1[6-9]|2\\d|3[0-1])(?:\\.\\d{1,3}){2})"
                        // IP address dotted notation octets
                        // excludes loopback network 0.0.0.0
                        // excludes reserved space >= 224.0.0.0
                        // excludes network & broacast addresses
                        // (first & last IP address of each class)
                        "(?:[1-9]\\d?|1\\d\\d|2[01]\\d|22[0-3])"
                        "(?:\\.(?:1?\\d{1,2}|2[0-4]\\d|25[0-5])){2}"
                        "(?:\\.(?:[1-9]\\d?|1\\d\\d|2[0-4]\\d|25[0-4]))"
                        "|"
                        // host name
                        "(?:(?:[a-z\\x{00a1}-\\x{ffff}0-9]+-?)*[a-z\\x{00a1}-\\x{ffff}0-9]+)"
                        // domain name
                        "(?:\\.(?:[a-z\\x{00a1}-\\x{ffff}0-9]+-?)*[a-z\\x{00a1}-\\x{ffff}0-9]+)*"
                        // TLD identifier
                        "(?:\\.(?:[a-z\\x{00a1}-\\x{ffff}]{2,}))"
                        ")"
                        // port number
                        "(?::\\d{2,5})?"
                        // resource path
                        "(?:/[^\\s]*)?"
                        "$");
    static QRegularExpression rx(urlRegExp, QRegularExpression::CaseInsensitiveOption);
    //return rx.match(url).hasMatch();

    return true;
}

bool CredentialsPage::isAllFieldNotEmpty()
{
    return !ui->edUrl->text().trimmed().isEmpty() &&
           !ui->edEmail->text().trimmed().isEmpty() &&
           !ui->edPassword->text().trimmed().isEmpty();
}
