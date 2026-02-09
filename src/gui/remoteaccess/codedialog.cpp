#include "codedialog.h"
#include "ui_codedialog.h"

#include "gui/customui/stylehelper.h"
#include "gui/customui/focusproxy.h"
#include "theme.h"

#include <QKeyEvent>
#include <QGraphicsDropShadowEffect>
#include <QLoggingCategory>

namespace {
const auto widgetStyle = QStringLiteral(":/res/login/codedialog.qss");
}

Q_LOGGING_CATEGORY(lcCodeDialog, "gui.codedialog", QtInfoMsg)

CodeDialogController::CodeDialogController(QObject *parent)
    : QObject(parent)
{
    codeInputEnabled.setBinding([this]() {
        return state.value() != CodeDialogState::Waiting;
    });

    btnAllowAccessVisible.setBinding([this]() {
        return state.value() == CodeDialogState::AllowAccess;
    });

    btnAllowAccessEnabled.setBinding([this]() {
        return isCodeValid() && errorState.value() == CodeErrorState::None;
    });

    btnResendCodeVisible.setBinding([this]() {
        return state.value() == CodeDialogState::Resend;
    });

    btnResendCodeEnabled.setBinding([this]() {
        return isCodeValid() && errorState.value() == CodeErrorState::None;
    });

    spinnerVisible.setBinding([this]() {
        return state.value() == CodeDialogState::Waiting;
    });

    errorString.setBinding([this]() {
        switch (errorState.value())
        {
        case CodeErrorState::None:
            return QStringLiteral("");
        case CodeErrorState::CodeInvalid:
            return tr("Incorrect code");
        case CodeErrorState::CodeExpired:
            return tr("Your code has expired");
        }
        return QStringLiteral("");
    });
}

void CodeDialogController::clearError()
{
    errorState.setValue(CodeErrorState::None);
}

CodeDialog::CodeDialog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CodeDialog)
    , controller_(new CodeDialogController(this))
{
    ui->setupUi(this);

    setStyleSheet(APP::StyleHelper::loadFileToString(widgetStyle));
    connect(APP::Theme::instance(), &APP::Theme::themeChanged, this, [this](bool isDark) {
        controller_->darkTheme.setValue(isDark);
    });

    // setWindowFlags(Qt::Tool|Qt::FramelessWindowHint|Qt::NoDropShadowWindowHint);
    setWindowFlags(Qt::FramelessWindowHint|Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
    // setWindowModality(Qt::WindowModal);

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
        ui->lblError->setVisible(controller_->errorState.value() != CodeErrorState::None);
        ui->lblError->setText(controller_->errorString.value());
        ui->lblError->setToolTip(controller_->errorTooltip.value());
        ui->codeInputWidget->setErrorState(controller_->errorState.value() != CodeErrorState::None);
    };
    auto updateThemeFunc = [this]() {
        APP::StyleHelper::setTheme(this, controller_->darkTheme.value());
        qCDebug(lcCodeDialog) << "isDark" << controller_->darkTheme.value();
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
    notifiers_.emplace_back(controller_->darkTheme.addNotifier(updateThemeFunc));

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
    controller_->darkTheme.setValue(APP::Theme::instance()->isDarkTheme());
    controller_->codeString.setValue(QStringLiteral(""));
    updateThemeFunc();

    installEventFilter(this);
    parent->installEventFilter(this);

    syncUI();
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

void CodeDialog::setError(CodeErrorState errorState)
{
    controller_->errorState.setValue(errorState);
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

bool CodeDialog::eventFilter(QObject *obj, QEvent *ev)
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

