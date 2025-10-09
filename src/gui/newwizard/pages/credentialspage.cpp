#include "credentialspage.h"
#include "ui_credentialspage.h"

#include "gui/customui/stylehelper.h"
#include "gui/customui/focusproxy.h"
#include "theme.h"


#include <QLineEdit>
#include <QRegularExpression>

namespace {
constexpr int fontSize = 16;
QPair<QString,QString> widgetStyle = {
    QStringLiteral(":/res/login/cred_page_light.qss"),
    QStringLiteral(":/res/login/cred_page_dark.qss")
};
QPair<QString,QString> eyeIcon = {
    QStringLiteral(":/res/login/eye_light.svg"),
    QStringLiteral(":/res/login/eye_dark.svg")
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
    connect(ui->btnSettings, &QPushButton::clicked, this, &CredentialsPage::settingsClicked);
    connect(ui->btnResetPass, &QPushButton::clicked, this, &CredentialsPage::resetPasswordClicked);

    ui->btnLogin->setMouseTracking(true);

    ui->edEmail->setPlaceholderText(tr("Email"));
    ui->edEmail->setFontPixelSize(fontSize);

    ui->edUrl->setPlaceholderText(tr("Connecting to"));
    ui->edUrl->setFontPixelSize(fontSize);

    ui->edPassword->setPlaceholderText(tr("Password"));
    ui->edPassword->setFontPixelSize(fontSize);
    ui->edPassword->setPasswordMode(true);

    connect(ui->edUrl, &InputWidget::textChanged, this, &CredentialsPage::onTextChanged);
    connect(ui->edEmail, &InputWidget::textChanged, this, &CredentialsPage::onTextChanged);
    connect(ui->edPassword, &InputWidget::textChanged, this, &CredentialsPage::onTextChanged);

    connect(ui->edUrl, &InputWidget::textEdited, this, &CredentialsPage::onTextEdited);
    connect(ui->edEmail, &InputWidget::textEdited, this, &CredentialsPage::onTextEdited);
    connect(ui->edPassword, &InputWidget::textEdited, this, &CredentialsPage::onTextEdited);

    connect(ui->edUrl, &InputWidget::editingFinished, this, &CredentialsPage::onEditingFinished);
    connect(ui->edEmail, &InputWidget::editingFinished, this, &CredentialsPage::onEditingFinished);
    connect(ui->edPassword, &InputWidget::editingFinished, this, &CredentialsPage::onEditingFinished);

    validateFormData();

    ui->btnLogin->installEventFilter(this);
    ui->btnLogin->setToolTip(tr("Enter a valid email address and password"));

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
    setStyleSheet(CUR::StyleHelper::loadFileToString(isDark ? widgetStyle.second : widgetStyle.first));

    ui->edPassword->setPasswordButtonImage(isDark ? eyeIcon.second : eyeIcon.first);
    update();
}

QString CredentialsPage::url() const
{
    return ui->edUrl->text();
}

QString CredentialsPage::email() const
{
    return ui->edEmail->text();
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

void CredentialsPage::onTextChanged(const QString& /*txt*/)
{
    validateFormData();
    showErrorMessage({});
}

void CredentialsPage::onTextEdited(const QString&/*txt*/)
{
    if (sender() == ui->edEmail)
        ui->edEmail->setErrorState(false);
    else if (sender() == ui->edUrl)
        ui->edUrl->setErrorState(false);
}

void CredentialsPage::onEditingFinished()
{
    qDebug() << "EditingFinished" << sender();
    if (!simpleEmailValidate(email())) {
        ui->edEmail->setErrorState(true, tr("Invalid email"));
    }
}

void CredentialsPage::validateFormData()
{
    ui->btnLogin->setEnabled(isAllFieldValid());
}

bool CredentialsPage::simpleEmailValidate(const QString& email)
{
    if (email.trimmed().isEmpty())
        return true;

    static QRegularExpression rx(QStringLiteral("^[0-9a-zA-Z]+([0-9a-zA-Z]*[-._+])*[0-9a-zA-Z]+@[0-9a-zA-Z]+([-.][0-9a-zA-Z]+)*([0-9a-zA-Z]*[.])[a-zA-Z]{2,6}$"),
                                 QRegularExpression::CaseInsensitiveOption);
    return rx.match(email).hasMatch();
}

bool CredentialsPage::simpleUrlValidate(const QString& url)
{
    if (url.trimmed().isEmpty())
        return true;

    QUrl urlValidator(url);
    qDebug() << urlValidator << urlValidator.isValid();
    return urlValidator.isValid();
}

bool CredentialsPage::isAllFieldValid()
{
    return !ui->edUrl->text().trimmed().isEmpty() &&
           !ui->edEmail->text().trimmed().isEmpty() &&
           !ui->edPassword->text().trimmed().isEmpty();
}
