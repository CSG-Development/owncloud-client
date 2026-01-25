#include "emailpage.h"
#include "ui_emailpage.h"

#include "gui/customui/stylehelper.h"
#include "gui/customui/focusproxy.h"
#include "theme.h"

#include <QLineEdit>
#include <configfile.h>

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
}

EmailPageController::EmailPageController(QObject *parent)
    : QObject(parent)
{
    canLogin.setBinding([this]() {
        return isEmailValid(email.value());
    });
    buttonTooltip.setBinding([this]() {
        auto s = canLogin.value() ? QStringLiteral("") : loginBtnTooltip;
        return s;
    });
    errorState.setBinding([this]() {
        const auto& currentEmail = email.value();
        return !canLogin.value() && !isFocused.value() && !currentEmail.trimmed().isEmpty();
    });
}

EmailPage::EmailPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EmailPage)
{
    controller_ = new EmailPageController(this);

    ui->setupUi(this);
    ui->frameErrorMessage->setVisible(false);

    setObjectName("emailPage");
    setMouseTracking(true);

    auto syncUI = [this]() {
        ui->btnLogin->setToolTip(controller_->buttonTooltip.value());
        ui->btnLogin->setEnabled(controller_->canLogin.value());
        ui->edEmail->setErrorState(controller_->errorState.value(), controller_->errorState.value() ? tr("Invalid email") : QStringLiteral(""));
        if (ui->edEmail->text() != controller_->email.value()) {
            QSignalBlocker blocker(ui->edEmail);
            ui->edEmail->setText(controller_->email.value());
        }
    };

    notifiers_.emplace_back(controller_->buttonTooltip.addNotifier(syncUI));
    notifiers_.emplace_back(controller_->canLogin.addNotifier(syncUI));
    notifiers_.emplace_back(controller_->errorState.addNotifier(syncUI));

    ui->btnLogin->setStyle(new FocusProxyStyle(ui->btnLogin));
    ui->btnCancel->setStyle(new FocusProxyStyle(ui->btnCancel));
    ui->btnSettings->setIconSize({20, 20});

    setAttribute(Qt::WA_TranslucentBackground, true);

    connect(ui->btnLogin, &QPushButton::clicked, this, [this] { emit loginClicked(email()); });

    connect(ui->btnCancel, &QPushButton::clicked, this, &EmailPage::cancelClicked);
    connect(ui->btnSettings, &QPushButton::clicked, this, &EmailPage::settingsClicked);

    ui->edEmail->setPlaceholderText(tr("Email"));
    ui->edEmail->setFontPixelSize(fontSize);

    connect(ui->edEmail, &InputWidget::textEdited, this, [this](const QString &txt) {
        controller_->email.setValue(txt);
    });

    connect(ui->edEmail, &InputWidget::focusChanged, this, [this](bool focused) {
        controller_->isFocused.setValue(focused);
    });

    // MacOS hover enable
    ui->btnLogin->setAttribute(Qt::WA_Hover, true);
    ui->btnCancel->setAttribute(Qt::WA_Hover, true);
    ui->btnSettings->setAttribute(Qt::WA_Hover, true);

    updateTheme();
    syncUI();
}

EmailPage::~EmailPage()
{
    delete ui;
    //delete noFocus;
}

void EmailPage::updateTheme()
{
    bool isDark = CUR::Theme::instance()->isDarkTheme();

    CUR::StyleHelper::invoke_setDarkTheme_recursive(this);

    // QToolButton "icon" property does not supported in qss
    ui->btnSettings->setIcon(isDark ? QIcon(settingsIcon.second) : QIcon(settingsIcon.first));
    setStyleSheet(CUR::StyleHelper::loadFileToString(isDark ? widgetStyle.second : widgetStyle.first));

    update();
}

void EmailPage::setEmail(const QString &email)
{
    controller_->email.setValue(email);
    emit emailChanged();
}

QString EmailPage::email() const
{
    return controller_->email.value();
}
