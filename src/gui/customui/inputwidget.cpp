#include "inputwidget.h"
#include "ui_inputwidget.h"
#include "stylehelper.h"
#include "theme.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QLabel>

namespace {
constexpr int btnIconSize = 21;

QPair<QString,QString> inputStyle = {
    QStringLiteral(":/res/inputwidget/inputwidget_light.qss"),
    QStringLiteral(":/res/inputwidget/inputwidget_dark.qss")
};
QPair<QString,QString> inputStyleError = {
    QStringLiteral(":/res/inputwidget/inputwidget_error_light.qss"),
    QStringLiteral(":/res/inputwidget/inputwidget_error_dark.qss")
};
}

InputWidget::InputWidget(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::InputWidget)
{
    ui->setupUi(this);

    promptLabel = new QLabel(this);
    promptLabel->setObjectName(QStringLiteral("promptLabel"));
    promptLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    promptLabel->setMaximumHeight(16);
    promptLabel->setVisible(false);

    ui->showPasswordButton->setVisible(false);
    ui->showPasswordButton->setCheckable(true);
    ui->showPasswordButton->setCursor(Qt::PointingHandCursor);
    ui->showPasswordButton->setIconSize(QSize(btnIconSize, btnIconSize));

    connect(ui->showPasswordButton, &QToolButton::clicked, this, [this] {
        ui->inputLineEdit->setEchoMode(ui->showPasswordButton->isChecked() ? QLineEdit::Normal : QLineEdit::Password);
    });

    connect(ui->inputLineEdit, &QLineEdit::textChanged, this, &InputWidget::onTextChanged);
    connect(ui->inputLineEdit, &QLineEdit::textEdited, this, &InputWidget::textEdited);
    connect(ui->inputLineEdit, &QLineEdit::editingFinished, this, &InputWidget::editingFinished);

    setErrorState(false);
    updatePromptPosition();
    setDarkTheme();
}

InputWidget::~InputWidget()
{
    delete ui;
}

QLineEdit *InputWidget::lineEdit() const
{
    return ui->inputLineEdit;
}

int InputWidget::fontPixelSize() const
{
    return ui->inputLineEdit->font().pixelSize();
}

void InputWidget::setPasswordMode(bool val)
{
    ui->inputLineEdit->setEchoMode(val ? QLineEdit::Password : QLineEdit::Normal);
    const QSignalBlocker b_(ui->showPasswordButton);
    ui->showPasswordButton->setChecked(!val);
    ui->showPasswordButton->setVisible(val);
}

void InputWidget::setPasswordButtonImage(const QString &val)
{
    ui->showPasswordButton->setIcon(QIcon(val));
}

void InputWidget::setPlaceholderText(const QString &str)
{
    ui->inputLineEdit->setPlaceholderText(str.trimmed());
    promptLabel->setText(str.trimmed());
    promptLabel->adjustSize();
}

QString InputWidget::text() const
{
    return ui->inputLineEdit->text();
}

void InputWidget::setText(const QString &val)
{
    ui->inputLineEdit->setText(val);
}

void InputWidget::setErrorState(bool enable, const QString& txt)
{
    errorState = enable;
    if (!errorState)
        ui->errorLabel->clear();
    else
        ui->errorLabel->setText(txt);
    updateStyles();
}

void InputWidget::setDarkTheme()
{
    isDark = CUR::Theme::instance()->isDarkTheme();
    updateStyles();
}

void InputWidget::setFontPixelSize(int val)
{
    auto font = ui->inputLineEdit->font();
    font.setPixelSize(val);
    ui->inputLineEdit->setFont(font);
}

void InputWidget::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
}

void InputWidget::resizeEvent(QResizeEvent */*event*/)
{
    updatePromptPosition();
}

void InputWidget::onTextChanged(const QString& str)
{
    if (!promptLabel->text().isEmpty())
        promptLabel->setVisible(!str.isEmpty());

    Q_EMIT textChanged(str);
}

void InputWidget::updatePromptPosition()
{
    promptLabel->move(ui->inputLineEdit->pos().x() + 12, rect().top());
}

void InputWidget::updateStyles()
{
    if (errorState)
        setStyleSheet(CUR::StyleHelper::loadFileToString(isDark ? inputStyleError.second : inputStyleError.first));
    else
        setStyleSheet(CUR::StyleHelper::loadFileToString(isDark ? inputStyle.second : inputStyle.first));

    update();
}
