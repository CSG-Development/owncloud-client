#include "errordialog.h"
#include "ui_errordialog.h"

#include "gui/customui/stylehelper.h"
#include "gui/customui/focusproxy.h"
#include "theme.h"

#include <QEvent>
#include <QKeyEvent>

namespace {
const auto widgetStyle = QStringLiteral(":/res/login/errordialog.qss");
}

Q_LOGGING_CATEGORY(lcErrorDialog, "gui.errordialog", QtInfoMsg)

ErrorDialogController::ErrorDialogController(QObject *parent)
    : QObject(parent)
{
    okBtnVisible.setBinding([this]() {
        return state.value() == ErrorDialogState::TooManyAttempts ||
               state.value() == ErrorDialogState::EmailNotRegistered;
    });
    retryBtnVisible.setBinding([this]() {
        return state.value() == ErrorDialogState::UnableToConnectInit ||
               state.value() == ErrorDialogState::UnableToConnectToken;
    });
    cancelBtnVisible.setBinding([this]() {
        return state.value() == ErrorDialogState::UnableToConnectInit ||
               state.value() == ErrorDialogState::UnableToConnectToken;
    });
    headerText.setBinding([this]() {
        switch (state.value()) {
            case ErrorDialogState::TooManyAttempts:
                return tr("Access to Personal Cloud");
            case ErrorDialogState::UnableToConnectInit:
            case ErrorDialogState::UnableToConnectToken:
                return tr("Unable to connect");
            case ErrorDialogState::EmailNotRegistered:
                return tr("Email not registered");
        }
        return QStringLiteral("");
    });
    contentText.setBinding([this]() {
        switch (state.value()) {
        case ErrorDialogState::TooManyAttempts:
            return tr("There have been too many attempts for access within the last minute. Please wait at least two minutes before retrying.");
        case ErrorDialogState::UnableToConnectInit:
        case ErrorDialogState::UnableToConnectToken:
            return tr("Could not reach your Personal Cloud.");
        case ErrorDialogState::EmailNotRegistered:
            return tr("This email isn’t authorized to access this device. Please contact the owner.");
        }
        return QStringLiteral("");
    });
}

ErrorDialog::ErrorDialog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ErrorDialog)
    , controller_(new ErrorDialogController(this))
{
    ui->setupUi(this);

    setStyleSheet(APP::StyleHelper::loadFileToString(widgetStyle));
    connect(APP::Theme::instance(), &APP::Theme::themeChanged, this, [this](bool isDark) {
        controller_->darkTheme.setValue(isDark);
    });

    setWindowFlags(Qt::FramelessWindowHint|Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);

    // MacOS hover enable
    ui->btnCancel->setAttribute(Qt::WA_Hover, true);
    ui->btnOk->setAttribute(Qt::WA_Hover, true);
    ui->btnRetry->setAttribute(Qt::WA_Hover, true);

    ui->btnCancel->setStyle(new FocusProxyStyle(ui->btnCancel));
    ui->btnOk->setStyle(new FocusProxyStyle(ui->btnOk));
    ui->btnRetry->setStyle(new FocusProxyStyle(ui->btnRetry));

    auto syncUI = [this]() {
        ui->btnCancel->setVisible(controller_->cancelBtnVisible.value());
        ui->btnRetry->setVisible(controller_->retryBtnVisible.value());
        ui->btnOk->setVisible(controller_->okBtnVisible.value());
        ui->lblHeader->setText(controller_->headerText.value());
        ui->lblText->setText(controller_->contentText.value());
    };
    auto updateThemeFunc = [this]() {
        APP::StyleHelper::setTheme(this, controller_->darkTheme.value());
        qCDebug(lcErrorDialog) << "isDark" << controller_->darkTheme.value();
    };
    notifiers_.emplace_back(controller_->state.addNotifier(syncUI));

    connect(ui->btnCancel, &QPushButton::clicked, this, [this] {
        qCDebug(lcErrorDialog) << "Cancel clicked";
        emit errorAction(ErrorAction::Cancel, controller_->state.value());
    });

    connect(ui->btnOk, &QPushButton::clicked, this, [this] {
        qCDebug(lcErrorDialog) << "OK clicked";
        emit errorAction(ErrorAction::Ok, controller_->state.value());
    });

    connect(ui->btnRetry, &QPushButton::clicked, this, [this] {
        qCDebug(lcErrorDialog) << "Retry clicked";
        emit errorAction(ErrorAction::Retry, controller_->state.value());
    });

    syncUI();
    controller_->darkTheme.setValue(APP::Theme::instance()->isDarkTheme());
    updateThemeFunc();

    installEventFilter(this);
    parent->installEventFilter(this);
}

ErrorDialog::~ErrorDialog()
{
    delete ui;
}

void ErrorDialog::setDialogState(ErrorDialogState state)
{
    qCDebug(lcErrorDialog) << "setDialogState" << ErrorDialogStateToStr(state);
    controller_->state.setValue(state);
}

void ErrorDialog::keyPressEvent(QKeyEvent *event)
{
    // Disable close on ESC key pressed
    if (event->key() != Qt::Key_Escape) {
        QWidget::keyPressEvent(event);
    }
}

bool ErrorDialog::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == parent()) {
        if (ev->type() == QEvent::Resize) {
            const auto parent_size = static_cast<QResizeEvent*>(ev)->size();
            resize(parent_size);
        }
        else if (ev->type() == QEvent::ChildAdded) {
            raise();
        }
    }
    else if (obj == this) {
        if (ev->type() == QEvent::Show && parent()) {
            const auto parent_widget = qobject_cast<QWidget*>(parent());
            resize(parent_widget->size());
        }
    }
    return QWidget::eventFilter(obj, ev);
}

