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

    connect(ui->btnAllowAccess, &QPushButton::clicked, this, &CodeDialog::allowAccessClicked);
    connect(ui->btnSkip, &QPushButton::clicked, this, &CodeDialog::skipClicked);
    connect(ui->btnResendCode, &QPushButton::clicked, this, [&] {
        showError({});
        emit resendCodeClicked();
    });

    connect(ui->codeInputWidget, &CodeInputWidget::codeChanged, this, [&] {
        ui->btnAllowAccess->setEnabled(ui->codeInputWidget->codeStr().length() == 6);
    });

    ui->spinner->setVisible(false);
    ui->btnAllowAccess->setEnabled(false);
    showError({});

    updateTheme();
}

CodeDialog::~CodeDialog()
{
    delete ui;
}

void CodeDialog::updateTheme()
{
    bool isDark = CUR::Theme::instance()->isDarkTheme();
    const QList<QWidget*> childrenList = findChildren<QWidget*>();
    for (auto* widget: childrenList) {
        if (widget->metaObject()->indexOfSlot("setDarkTheme()") != -1) {
            QMetaObject::invokeMethod(widget, "setDarkTheme");
        }
    }
    setStyleSheet(CUR::StyleHelper::loadFileToString(isDark ? widgetStyle.second : widgetStyle.first));

    update();
}

void CodeDialog::showError(const QString &txt)
{
    errorState_ = !txt.isEmpty();
    ui->codeInputWidget->setErrorState(errorState_);

    ui->lblError->setText(txt);

    if (errorState_) {
        ui->lblError->setVisible(true);
        ui->btnResendCode->setVisible(true);
        ui->btnAllowAccess->setVisible(false);
    }
    else {
        ui->lblError->setVisible(false);
        ui->btnResendCode->setVisible(false);
        ui->btnAllowAccess->setVisible(true);
    }
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
