#include "combowidget.h"
#include "ui_combowidget.h"

#include "stylehelper.h"
#include "popupcombowidget.h"
#include "theme.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QLabel>
#include <QAbstractItemView>
#include <QMouseEvent>

namespace {
QPair<QString,QString> inputStyle = {
    QStringLiteral(":/res/combowidget/combowidget_light.qss"),
    QStringLiteral(":/res/combowidget/combowidget_dark.qss")
};
QPair<QString,QString> inputStyleError = {
    QStringLiteral(":/res/combowidget/combowidget_error_light.qss"),
    QStringLiteral(":/res/combowidget/combowidget_error_dark.qss")
};
QPair<QString,QString> arrowButtonLight = {
    QStringLiteral(":/res/combowidget/triangle_down.svg"),
    QStringLiteral(":/res/combowidget/triangle_up.svg")
};
QPair<QString,QString> arrowButtonDark = {
    QStringLiteral(":/res/combowidget/triangle_down.svg"),
    QStringLiteral(":/res/combowidget/triangle_up.svg")
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

    blockMouseTimer.setSingleShot(true);

    ui->arrowButton->setCursor(Qt::PointingHandCursor);

    popup = new PopupComboWidget(this);
    popup->setVisible(false);
    popup->setAnchorWidget(ui->inputFrame);

    connect(popup, &PopupComboWidget::clickedOutside, this, [&] {
        popup->hide();
        updateButtonIcon();
        //blockMouseTimer.start(QApplication::doubleClickInterval());
        blockMouseTimer.start(1000);

        auto selected = popup->selectedDevice();
        if (selected) {
            selectedDevice = selected;
            ui->lineEdit->setText(selectedDevice->certificateCommonName);
        }
    });

    connect(ui->arrowButton, &QToolButton::clicked, this, [this] {
        popup->updateAndShow();
        updateButtonIcon();
    });

    // MacOS hover enable
    ui->arrowButton->setAttribute(Qt::WA_Hover, true);

    connect(ui->lineEdit, &QLineEdit::textChanged, this, &ComboWidget::onTextChanged);
    ui->lineEdit->installEventFilter(this);
    ui->lineEdit->setReadOnly(true);

    setErrorState(false);
    updatePromptPosition();
    setDarkTheme();
}

ComboWidget::~ComboWidget()
{
    delete ui;
}

void ComboWidget::setPlaceholderText(const QString &str)
{
    ui->lineEdit->setPlaceholderText(str.trimmed());
    promptLabel->setText(str.trimmed());
    promptLabel->adjustSize();
}

QString ComboWidget::text() const
{
    return ui->lineEdit->text();
}

void ComboWidget::setText(const QString &val)
{
    ui->lineEdit->setText(val);
}

void ComboWidget::setItems(const QList<Device> &list)
{
    QList<Device> tmpitems(list);
    qSwap(tmpitems, deviceList);
    popup->setItems(deviceList);

    if (!list.isEmpty() && ui->lineEdit->text().isEmpty()) {
        selectedDevice = list.first();
        ui->lineEdit->setText(selectedDevice->certificateCommonName);
    }
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
    if (watched == ui->lineEdit) {
        if (event->type() == QEvent::ToolTip) {
            //ui->lineEdit->setToolTip(ui->lineEdit->text().isEmpty() ? QStringLiteral("") : currentDevice()->host);
        }
    }

    return QFrame::eventFilter(watched, event);
}

void ComboWidget::setDarkTheme()
{
    isDark = CUR::Theme::instance()->isDarkTheme();
    popup->setDarkTheme(isDark);
    updateStyles();
}

void ComboWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (blockMouseTimer.isActive()) {
            event->ignore();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void ComboWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (blockMouseTimer.isActive()) {
            event->ignore();
            return;
        }
    }
    QWidget::mouseReleaseEvent(event);
}

void ComboWidget::setFontPixelSize(int val)
{
    auto font = ui->lineEdit->font();
    font.setPixelSize(val);
    ui->lineEdit->setFont(font);
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
    if (!promptLabel->text().isEmpty()) {
        promptLabel->setVisible(!str.isEmpty());
    }

    Q_EMIT textChanged(str);
}

void ComboWidget::updatePromptPosition()
{
    promptLabel->move(ui->lineEdit->pos().x() + 12, rect().top());
}

void ComboWidget::updateStyles()
{
    if (errorState)
        setStyleSheet(CUR::StyleHelper::loadFileToString(isDark ? inputStyleError.second : inputStyleError.first));
    else
        setStyleSheet(CUR::StyleHelper::loadFileToString(isDark ? inputStyle.second : inputStyle.first));

    style()->unpolish(this);
    style()->polish(this);

    updateButtonIcon();
}

void ComboWidget::updateButtonIcon()
{
    if (isDark) {
        ui->arrowButton->setIcon(popup->isVisible() ? QIcon(arrowButtonDark.second) : QIcon(arrowButtonDark.first));
    }
    else {
        ui->arrowButton->setIcon(popup->isVisible() ? QIcon(arrowButtonLight.second) : QIcon(arrowButtonLight.first));
    }
    update();
}
