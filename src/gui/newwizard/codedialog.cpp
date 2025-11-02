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
    : QDialog(parent)
    , ui(new Ui::CodeDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Tool|Qt::FramelessWindowHint|Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);

    auto noFocus = new FocusProxyStyle;
    ui->btnAllowAccess->setStyle(noFocus);
    ui->btnSkip->setStyle(noFocus);

    connect(ui->btnAllowAccess, &QPushButton::clicked, this, [&] {
        accept();
    });

    connect(ui->btnSkip, &QPushButton::clicked, this, [&] {
        reject();
    });

    connect(ui->widget, &CodeInputWidget::codeChanged, this, [&] {
        ui->btnAllowAccess->setEnabled(ui->widget->codeStr().length() == 6);
    });

    ui->spinner->setVisible(false);
    ui->btnAllowAccess->setEnabled(false);

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

QString CodeDialog::getCode() const
{
    return ui->widget->codeStr();
}

void CodeDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() != Qt::Key_Escape) {
        QDialog::keyPressEvent(event);
    }
}
