#include "combowidget.h"
#include "ui_combowidget.h"

#include "stylehelper.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QLabel>

namespace {
QPair<QString,QString> inputStyle = {
    QStringLiteral(":/res/combowidget/combowidget_light.qss"),
    QStringLiteral(":/res/combowidget/combowidget_dark.qss")
};
QPair<QString,QString> inputStyleError = {
    QStringLiteral(":/res/combowidget/combowidget_error_light.qss"),
    QStringLiteral(":/res/combowidget/combowidget_error_dark.qss")
};
}

ComboWidget::ComboWidget(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::ComboWidget)
{
    ui->setupUi(this);

    promptLabel = new QLabel(this);
    promptLabel->setObjectName(QStringLiteral("promptLabel"));
    promptLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    promptLabel->setMaximumHeight(16);
    promptLabel->setVisible(false);

    ui->arrowButton->setCursor(Qt::PointingHandCursor);
    ui->arrowButton->setVisible(false);

    connect(ui->arrowButton, &QToolButton::clicked, this, [this] {
    });

    connect(ui->comboBox, &QComboBox::currentTextChanged, this, &ComboWidget::onTextChanged);
    //ui->comboBox->addItem("Item 1");
    //ui->comboBox->addItem("Item 2");
    //ui->comboBox->addItem("Very very very very very very very long item");

    ui->comboBox->installEventFilter(this);

    setErrorState(false);
    updatePromptPosition();
    setDarkTheme();
}

ComboWidget::~ComboWidget()
{
    delete ui;
}

QComboBox* ComboWidget::comboBox() const
{
    return ui->comboBox;
}

int ComboWidget::fontPixelSize() const
{
    return ui->comboBox->font().pixelSize();
}

void ComboWidget::setPlaceholderText(const QString &str)
{
    ui->comboBox->setPlaceholderText(str.trimmed());
    promptLabel->setText(str.trimmed());
    promptLabel->adjustSize();
}

QString ComboWidget::text() const
{
    return ui->comboBox->currentText();
}

void ComboWidget::setText(const QString &val)
{
    ui->comboBox->setCurrentText(val);
}

void ComboWidget::setErrorState(bool enable, const QString& txt)
{
    errorState = enable;
    if (!errorState)
        ui->errorLabel->clear();
    else
        ui->errorLabel->setText(txt);
    updateStyles();
}

bool ComboWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->comboBox) {
        if (event->type() == QEvent::FocusIn) {
            Q_EMIT focusReceived();
        }
        else if (event->type() == QEvent::FocusOut) {
            Q_EMIT focusLost();
        }
    }
    return QFrame::eventFilter(watched, event);
}

void ComboWidget::setDarkTheme()
{
    isDark = false;//CUR::Theme::instance()->isDarkTheme();
    updateStyles();
}

void ComboWidget::setFontPixelSize(int val)
{
    auto font = ui->comboBox->font();
    font.setPixelSize(val);
    ui->comboBox->setFont(font);
}

void ComboWidget::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
}

void ComboWidget::resizeEvent(QResizeEvent */*event*/)
{
    updatePromptPosition();
}

void ComboWidget::onTextChanged(const QString& str)
{
    if (!promptLabel->text().isEmpty())
        promptLabel->setVisible(!str.isEmpty());

    Q_EMIT textChanged(str);
}

void ComboWidget::updatePromptPosition()
{
    promptLabel->move(ui->comboBox->pos().x() + 12, rect().top());
}

void ComboWidget::updateStyles()
{
    if (errorState)
        setStyleSheet(CUR::StyleHelper::loadFileToString(isDark ? inputStyleError.second : inputStyleError.first));
    else
        setStyleSheet(CUR::StyleHelper::loadFileToString(isDark ? inputStyle.second : inputStyle.first));

    update();
}
