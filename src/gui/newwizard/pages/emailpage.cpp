#include "emailpage.h"
#include "ui_emailpage.h"

#include "gui/customui/stylehelper.h"
#include "gui/customui/focusproxy.h"
#include "gui/customui/dimwidget.h"
#include "codedialog.h"
#include "theme.h"

#include <QLineEdit>
#include <QRegularExpression>

namespace {
constexpr int fontSize = 16;
QPair<QString,QString> widgetStyle = {
    QStringLiteral(":/res/login/cred_page_light.qss"),
    QStringLiteral(":/res/login/cred_page_dark.qss")
};
QPair<QString,QString> settingsIcon = {
    QStringLiteral(":/res/login/gear_light.svg"),
    QStringLiteral(":/res/login/gear_dark.svg")
};
const QString loginBtnTooltip = QObject::tr("Enter a valid email address");
constexpr int code_expire_seconds = 10 * 60;     // 10 min
}

EmailPage::EmailPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EmailPage)
{
    ui->setupUi(this);
    setObjectName("emailPage");
    setMouseTracking(true);

    auto noFocus = new FocusProxyStyle;
    ui->btnLogin->setStyle(noFocus);
    ui->btnCancel->setStyle(noFocus);
    ui->btnSettings->setIconSize({20, 20});

    setAttribute(Qt::WA_TranslucentBackground, true);

    dim = new DimWidget(this);
    dim->setVisible(false);

    connect(ui->btnLogin, &QPushButton::clicked, this, [&] {
        Q_EMIT loginClicked(email());
    });
    connect(ui->btnCancel, &QPushButton::clicked, this, &EmailPage::cancelClicked);
    connect(ui->btnSettings, &QPushButton::clicked, this, &EmailPage::settingsClicked);

    ui->btnLogin->setMouseTracking(true);

    ui->edEmail->setPlaceholderText(tr("Email"));
    ui->edEmail->setFontPixelSize(fontSize);

    connect(ui->edEmail, &InputWidget::textEdited, this, &EmailPage::onTextEdited);

    connect(ui->edEmail, &InputWidget::focusReceived, this, [&] {
        ui->edEmail->setErrorState(false);
    });
    connect(ui->edEmail, &InputWidget::focusLost, this, [&] {
        if (!simpleEmailValidate(email())) {
            ui->edEmail->setErrorState(true, tr("Invalid email"));
            ui->btnLogin->setEnabled(false);
        }
    });

    validateFormData();

    ui->btnLogin->installEventFilter(this);
    ui->btnLogin->setToolTip(loginBtnTooltip);
    showErrorMessage({});

    codeExpireCheckTimer.setInterval(1000);
    connect(&codeExpireCheckTimer, &QTimer::timeout, this, &EmailPage::onCodeExpireCheckTimer);

    updateTheme();
}

EmailPage::~EmailPage()
{
    delete ui;
}

bool EmailPage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->btnLogin) {
        if (event->type() == QEvent::EnabledChange) {
            if (ui->btnLogin->isEnabled()) {
                ui->btnLogin->setToolTip({});
            }
            else {
                ui->btnLogin->setToolTip(loginBtnTooltip);
            }

        }
    }
    return QWidget::eventFilter(watched, event);
}

void EmailPage::updateTheme()
{
    bool isDark = CUR::Theme::instance()->isDarkTheme();

    const QList<QWidget*> childrenList = findChildren<QWidget*>();
    for (auto* widget: childrenList) {
        if (widget->metaObject()->indexOfSlot("setDarkTheme()") != -1) {
            QMetaObject::invokeMethod(widget, "setDarkTheme");
        }
    }

    // QToolButton "icon" property does not supported in qss
    ui->btnSettings->setIcon(isDark ? QIcon(settingsIcon.second) : QIcon(settingsIcon.first));
    setStyleSheet(CUR::StyleHelper::loadFileToString(isDark ? widgetStyle.second : widgetStyle.first));

    update();
}

QString EmailPage::email() const
{
    return ui->edEmail->text();
}

void EmailPage::showErrorMessage(const QString& msg)
{
    ui->frameErrorMessage->setVisible(!msg.isEmpty());
    ui->lblErrorText->setText(msg);
}

void EmailPage::showInvalidCredentialsError()
{
    ui->edEmail->setErrorState(true, {});
    ui->btnLogin->setEnabled(false);
}

void EmailPage::showCodeDialog()
{
    codeExpireTime = QDateTime::currentDateTime().addSecs(code_expire_seconds);
    codeExpireCheckTimer.start();

    dim->setVisible(true);
    CodeDialog dlg(this);
    int res = dlg.exec();
    dim->setVisible(false);

    if (res == QDialog::Accepted) {
        emit codeEntered(dlg.getCode());
    }
    else {
        emit codeSkipped();
    }
}

void EmailPage::onTextEdited(const QString&/*txt*/)
{
    if (sender() == ui->edEmail) {
        ui->edEmail->setErrorState(false);
    }

    validateFormData();
    showErrorMessage({});
}

void EmailPage::onCodeExpireCheckTimer()
{
    if (QDateTime::currentDateTime() > codeExpireTime) {
        codeExpireCheckTimer.stop();
        emit codeExpired();
    }
}

void EmailPage::validateFormData()
{
    bool valid = isAllFieldNotEmpty() &&
                 simpleEmailValidate(email());

    ui->btnLogin->setEnabled(valid);
}

bool EmailPage::simpleEmailValidate(const QString& email)
{
    if (email.trimmed().isEmpty())
        return true;

    static QRegularExpression rx(QStringLiteral("^[0-9a-zA-Z]+([0-9a-zA-Z]*[-._+])*[0-9a-zA-Z]+@[0-9a-zA-Z]+([-.][0-9a-zA-Z]+)*([0-9a-zA-Z]*[.])[a-zA-Z]{2,6}$"),
                                 QRegularExpression::CaseInsensitiveOption);
    return rx.match(email).hasMatch();
}

bool EmailPage::isAllFieldNotEmpty()
{
    return !ui->edEmail->text().trimmed().isEmpty();
}
