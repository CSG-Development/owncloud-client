#include "emailpage.h"
#include "ui_emailpage.h"

#include "gui/customui/stylehelper.h"
#include "gui/customui/focusproxy.h"
#include "theme.h"
#include "configfile.h"

#include <QLineEdit>
#include <QLoggingCategory>

namespace {
constexpr int fontSize = 16;
QPair<QString,QString> settingsIcon = {
    QStringLiteral(":/res/login/gear_light.svg"),
    QStringLiteral(":/res/login/gear_dark.svg")
};
const QString loginBtnTooltip = QObject::tr("Enter a valid email address");
}

Q_LOGGING_CATEGORY(lcEmailPage, "gui.emailpage")


EmailPageController::EmailPageController(QObject *parent)
    : QObject(parent)
{
    canLogin.setBinding([this]() {
        return isEmailValid(email.value()) && !notRegistered.value();
    });

    buttonTooltip.setBinding([this]() {
        auto s = canLogin.value() ? QStringLiteral("") : loginBtnTooltip;
        return s;
    });

    errorState.setBinding([this]() {
        const auto& currentEmail = email.value();
        if (notRegistered && !isFocused.value())
            return EmailErrorState::NotAllowed;
        if (!canLogin.value() && !isFocused.value() && !currentEmail.trimmed().isEmpty())
            return EmailErrorState::InvalidEmail;
        return EmailErrorState::NoError;
    });

    errorMessage.setBinding([this]() {
        switch (errorState.value())
        {
        case EmailErrorState::NoError:
            return QStringLiteral("");
        case EmailErrorState::InvalidEmail:
            return tr("Invalid email");
        case EmailErrorState::NotAllowed:
            return tr("Not allowed. Contact the device owner.");
        }
        return QStringLiteral("");
    });
}

EmailPage::EmailPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EmailPage)
{
    controller_ = new EmailPageController(this);

    ui->setupUi(this);
    ui->frameErrorMessage->setVisible(false);

    setStyleSheet(APP::StyleHelper::loadFileToString(QStringLiteral(":/res/login/cred_page.qss")));

    setObjectName("emailPage");
    setMouseTracking(true);

    auto syncUI = [this]() {
        ui->btnLogin->setToolTip(controller_->buttonTooltip.value());
        ui->btnLogin->setEnabled(controller_->canLogin.value());
        // ui->edEmail->setErrorState(controller_->errorState.value(), controller_->errorState.value() ? tr("Invalid email") : QStringLiteral(""));
        ui->edEmail->setErrorState(controller_->errorState.value() != EmailErrorState::NoError,
                                   controller_->errorMessage.value());
        if (ui->edEmail->text() != controller_->email.value()) {
            QSignalBlocker blocker(ui->edEmail);
            ui->edEmail->setText(controller_->email.value());
        }
    };
    auto updateThemeFunc = [this]() {
        updateTheme();
    };

    notifiers_.emplace_back(controller_->buttonTooltip.addNotifier(syncUI));
    notifiers_.emplace_back(controller_->canLogin.addNotifier(syncUI));
    notifiers_.emplace_back(controller_->errorState.addNotifier(syncUI));
    notifiers_.emplace_back(controller_->darkTheme.addNotifier(updateThemeFunc));

    controller_->darkTheme.setValue(APP::Theme::instance()->isDarkTheme());

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
        if (controller_->isFocused.value()) {
            controller_->notRegistered.setValue(false);
        }
    });

    // MacOS hover enable
    ui->btnLogin->setAttribute(Qt::WA_Hover, true);
    ui->btnCancel->setAttribute(Qt::WA_Hover, true);
    ui->btnSettings->setAttribute(Qt::WA_Hover, true);

    updateThemeFunc();
    syncUI();
    ui->btnSettings->setVisible(false);
}

EmailPage::~EmailPage()
{
    delete ui;
    //delete noFocus;
}

void EmailPage::updateTheme()
{
    qCDebug(lcEmailPage) << controller_->darkTheme.value();
    APP::StyleHelper::invoke_setDarkTheme_recursive(this);

    // QToolButton "icon" property does not supported in qss
    ui->btnSettings->setIcon(controller_->darkTheme.value() ? QIcon(settingsIcon.second) : QIcon(settingsIcon.first));
    APP::StyleHelper::setTheme(this, controller_->darkTheme.value());

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
