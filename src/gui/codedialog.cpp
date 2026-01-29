#include "codedialog.h"
#include "ui_codedialog.h"

#include "gui/customui/stylehelper.h"
#include "gui/customui/focusproxy.h"

#include "theme.h"

#include <QKeyEvent>
#include <QGraphicsDropShadowEffect>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcCodeDialog, "gui.codedialog", QtInfoMsg)

CodeDialogController::CodeDialogController(QObject *parent)
    : QObject(parent)
{
    errorState.setBinding([this]() {
        return !errorString.value().isEmpty();
    });

    codeInputEnabled.setBinding([this]() {
        return state.value() != CodeDialogState::Waiting;
    });

    btnAllowAccessVisible.setBinding([this]() {
        return state.value() == CodeDialogState::AllowAccess;
    });

    btnAllowAccessEnabled.setBinding([this]() {
        return isCodeValid();
    });

    btnResendCodeVisible.setBinding([this]() {
        return state.value() == CodeDialogState::Resend;
    });

    btnResendCodeEnabled.setBinding([this]() {
        return state.value() == CodeDialogState::Resend && isCodeValid();
    });

    spinnerVisible.setBinding([this]() {
        return state.value() == CodeDialogState::Waiting;
    });
}

void CodeDialogController::clearError()
{
    errorString.setValue({});
    errorTooltip.setValue({});
}

CodeDialog::CodeDialog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CodeDialog)
    , controller_(new CodeDialogController(this))
{
    ui->setupUi(this);

    setStyleSheet(CUR::StyleHelper::loadFileToString(QStringLiteral(":/res/login/code_dialog.qss")));
    connect(CUR::Theme::instance(), &CUR::Theme::themeChanged, this, [this](bool isDark) {
        controller_->darkTheme.setValue(isDark);
    });

    setWindowFlags(Qt::Tool|Qt::FramelessWindowHint|Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
    setWindowModality(Qt::WindowModal);

    // MacOS hover enable
    ui->btnAllowAccess->setAttribute(Qt::WA_Hover, true);
    ui->btnSkip->setAttribute(Qt::WA_Hover, true);
    ui->btnResendCode->setAttribute(Qt::WA_Hover, true);

    ui->btnAllowAccess->setStyle(new FocusProxyStyle(ui->btnAllowAccess));
    ui->btnSkip->setStyle(new FocusProxyStyle(ui->btnSkip));
    ui->btnResendCode->setStyle(new FocusProxyStyle(ui->btnResendCode));

    auto syncUI = [this]() {
        ui->codeInputWidget->setEnabled(controller_->codeInputEnabled.value());
        ui->btnAllowAccess->setVisible(controller_->btnAllowAccessVisible.value());
        ui->btnAllowAccess->setEnabled(controller_->btnAllowAccessEnabled.value());
        ui->btnResendCode->setVisible(controller_->btnResendCodeVisible.value());
        ui->btnResendCode->setEnabled(controller_->btnResendCodeEnabled.value());
        ui->spinner->setVisible(controller_->spinnerVisible.value());
        ui->lblError->setVisible(controller_->errorState.value());
        ui->lblError->setText(controller_->errorString.value());
        ui->lblError->setToolTip(controller_->errorTooltip.value());
        ui->codeInputWidget->setErrorState(controller_->errorState.value());
    };
    auto updateTheme = [this]() {
        CUR::StyleHelper::setTheme(this, controller_->darkTheme.value());
    };

    notifiers_.emplace_back(controller_->errorState.addNotifier(syncUI));
    notifiers_.emplace_back(controller_->errorString.addNotifier(syncUI));
    notifiers_.emplace_back(controller_->errorTooltip.addNotifier(syncUI));
    notifiers_.emplace_back(controller_->codeInputEnabled.addNotifier(syncUI));
    notifiers_.emplace_back(controller_->btnAllowAccessVisible.addNotifier(syncUI));
    notifiers_.emplace_back(controller_->btnAllowAccessEnabled.addNotifier(syncUI));
    notifiers_.emplace_back(controller_->btnResendCodeVisible.addNotifier(syncUI));
    notifiers_.emplace_back(controller_->btnResendCodeEnabled.addNotifier(syncUI));
    notifiers_.emplace_back(controller_->spinnerVisible.addNotifier(syncUI));
    notifiers_.emplace_back(controller_->state.addNotifier(syncUI));
    notifiers_.emplace_back(controller_->darkTheme.addNotifier(updateTheme));

    connect(ui->btnAllowAccess, &QPushButton::clicked, this, [this] {
        qCDebug(lcCodeDialog) << "Allow clicked";
        setDialogState(CodeDialogState::Waiting);
        emit codeAction(CodeAction::Entered, controller_->codeString.value());
    });

    connect(ui->btnSkip, &QPushButton::clicked, this, [this] {
        qCDebug(lcCodeDialog) << "Skip clicked";
        controller_->clearError();
        emit codeAction(CodeAction::Skip);
    });

    connect(ui->btnResendCode, &QPushButton::clicked, this, [this] {
        qCDebug(lcCodeDialog) << "Resend clicked";
        controller_->clearError();
        setDialogState(CodeDialogState::Waiting);
        emit codeAction(CodeAction::Resend);
    });

    connect(ui->codeInputWidget, &CodeInputWidget::codeChanged, this, [this] {
        controller_->clearError();
        controller_->codeString.setValue(ui->codeInputWidget->codeStr());
    });

    connect(ui->codeInputWidget, &CodeInputWidget::focusGained, this, [this] {
        controller_->clearError();
    });

    setDialogState(CodeDialogState::AllowAccess);
    syncUI();
    controller_->darkTheme.setValue(CUR::Theme::instance()->isDarkTheme());
}

CodeDialog::~CodeDialog()
{
    delete ui;
}

void CodeDialog::reset()
{
    clearCode();
    controller_->clearError();
    setDialogState(CodeDialogState::AllowAccess);
}

void CodeDialog::setDialogState(CodeDialogState state)
{
    qCDebug(lcCodeDialog) << "setDialogState" << CodeDialogStateToStr(state);
    controller_->state.setValue(state);
}

void CodeDialog::setError(CodeDialogState state, const QString& errorStr, const QString& errorTooltip)
{
    setDialogState(state);
    if (state == CodeDialogState::Resend)   // "Your code has expired"
        clearCode();
    controller_->errorString.setValue(errorStr);
    controller_->errorTooltip.setValue(errorTooltip);
    // "Incorrect code"
    // "Server error. Try again"
}

QString CodeDialog::getCode() const
{
    return ui->codeInputWidget->codeStr();
}

void CodeDialog::clearCode()
{
    ui->codeInputWidget->clearCode();
}

void CodeDialog::keyPressEvent(QKeyEvent *event)
{
    // Disable close on ESC key pressed
    if (event->key() != Qt::Key_Escape) {
        QWidget::keyPressEvent(event);
    }
}

QString CodeDialog::CodeDialogStateToStr(CodeDialogState state)
{
    QMap<CodeDialogState, QString> map = {
        {CodeDialogState::Waiting, QStringLiteral("Waiting")},
        {CodeDialogState::AllowAccess, QStringLiteral("AllowAccess")},
        {CodeDialogState::Resend, QStringLiteral("Resend")},
    };
    if (map.contains(state))
        return map[state];
    return {};
}

