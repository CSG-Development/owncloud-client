#include "codedialog.h"
#include "ui_codedialog.h"

#include "gui/customui/stylehelper.h"
#include "gui/customui/focusproxy.h"

#include "theme.h"

#include <QKeyEvent>


namespace {
QPair<QString,QString> widgetStyle = {
    QStringLiteral(":/res/login/code_dialog_light.qss"),
    QStringLiteral(":/res/login/code_dialog_dark.qss")
};
}

CodeDialog::CodeDialog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CodeDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Tool|Qt::FramelessWindowHint|Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
    setWindowModality(Qt::WindowModal);

    auto noFocus = new FocusProxyStyle;
    ui->btnAllowAccess->setStyle(noFocus);
    ui->btnSkip->setStyle(noFocus);
    ui->btnResendCode->setStyle(noFocus);

    connect(ui->btnAllowAccess, &QPushButton::clicked, this, &CodeDialog::onAllowAccessClicked);
    connect(ui->btnSkip, &QPushButton::clicked, this, &CodeDialog::skipClicked);
    connect(ui->btnResendCode, &QPushButton::clicked, this, &CodeDialog::onResendCodeClicked);

    connect(ui->codeInputWidget, &CodeInputWidget::codeChanged, this, [&] {
        clearError();
        checkCodeValid();
    });

    ui->btnAllowAccess->setEnabled(false);
    ui->btnResendCode->setEnabled(false);

    setDialogState(CodeDialogState::Startup);

    updateTheme();
}

CodeDialog::~CodeDialog()
{
    delete ui;
}

void CodeDialog::updateTheme()
{
    bool isDark = CUR::Theme::instance()->isDarkTheme();
    CUR::StyleHelper::invoke_setDarkTheme_recursive(this);
    setStyleSheet(CUR::StyleHelper::loadFileToString(isDark ? widgetStyle.second : widgetStyle.first));

    update();
}

void CodeDialog::setDialogState(CodeDialogState state)
{
    qDebug() << "setDialogState" << CodeDialogStateToStr(state);
    state_ = state;
    switch (state_)
    {
        case CodeDialogState::Startup:
            ui->codeInputWidget->setEnabled(true);
            ui->btnAllowAccess->setVisible(true);
            ui->btnResendCode->setVisible(false);
            ui->spinner->setVisible(false);
            clearError();
            break;

        case CodeDialogState::Waiting:
            ui->codeInputWidget->setEnabled(false);
            ui->btnAllowAccess->setVisible(false);
            ui->btnResendCode->setVisible(false);
            ui->spinner->setVisible(true);
            clearError();
            break;

        case CodeDialogState::AllowAccess:
            ui->codeInputWidget->setEnabled(true);
            ui->btnAllowAccess->setVisible(true);
            ui->btnResendCode->setVisible(false);
            ui->spinner->setVisible(false);
            break;

        case CodeDialogState::Resend:
            ui->codeInputWidget->clearCode();
            ui->codeInputWidget->setEnabled(true);
            ui->btnAllowAccess->setVisible(false);
            ui->btnResendCode->setVisible(true);
            ui->btnResendCode->setEnabled(true);
            ui->spinner->setVisible(false);
            break;
    }
}

void CodeDialog::showCodeExpiredError()
{
    errorState_ = true;
    ui->codeInputWidget->setErrorState(errorState_);
    clearCode();
    ui->lblError->setText(tr("Your code has expired"));
    ui->lblError->setVisible(true);
    setDialogState(CodeDialogState::Resend);
}

void CodeDialog::showInvalidCodeError()
{
    errorState_ = true;
    ui->codeInputWidget->setErrorState(errorState_);
    ui->lblError->setText(tr("Incorrect code"));
    ui->lblError->setVisible(true);
    ui->btnAllowAccess->setEnabled(false);
    setDialogState(CodeDialogState::AllowAccess);
}

void CodeDialog::showServerError()
{
    errorState_ = true;
    ui->codeInputWidget->setErrorState(errorState_);
    ui->lblError->setText(tr("Server error. Try again"));
    ui->lblError->setVisible(true);
    ui->btnAllowAccess->setEnabled(false);
    setDialogState(CodeDialogState::AllowAccess);
}

void CodeDialog::clearError()
{
    errorState_ = false;
    ui->codeInputWidget->setErrorState(errorState_);
    ui->lblError->clear();
    ui->lblError->setVisible(false);
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
    if (event->key() != Qt::Key_Escape) {
        QWidget::keyPressEvent(event);
    }
}

void CodeDialog::onAllowAccessClicked()
{
    ui->btnAllowAccess->setVisible(false);
    ui->spinner->setVisible(true);
    emit allowAccessClicked();
}

void CodeDialog::onResendCodeClicked()
{
    clearError();
    ui->btnResendCode->setVisible(false);
    ui->spinner->setVisible(true);
    emit resendCodeClicked();
}

void CodeDialog::showResendButton()
{
    clearError();
    ui->spinner->setVisible(false);
    ui->btnAllowAccess->setVisible(false);
    ui->btnResendCode->setVisible(true);
}

void CodeDialog::showAllowButton()
{
    clearError();
    ui->spinner->setVisible(false);
    ui->btnAllowAccess->setVisible(true);
    ui->btnResendCode->setVisible(false);
}

void CodeDialog::checkCodeValid()
{
    ui->btnAllowAccess->setEnabled(ui->codeInputWidget->codeStr().length() == 6);
    ui->btnResendCode->setEnabled(ui->codeInputWidget->codeStr().length() == 6);
}

QString CodeDialog::CodeDialogStateToStr(CodeDialogState state)
{
    QMap<CodeDialogState, QString> map = {
        {CodeDialogState::Startup, QStringLiteral("Startup")},
        {CodeDialogState::Waiting, QStringLiteral("Waiting")},
        {CodeDialogState::AllowAccess, QStringLiteral("AllowAccess")},
        {CodeDialogState::Resend, QStringLiteral("Resend")},
    };
    if (map.contains(state))
        return map[state];
    return {};
}
