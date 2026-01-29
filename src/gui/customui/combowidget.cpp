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
    QStringLiteral(":/res/combowidget/triangle_down_light.svg"),
    QStringLiteral(":/res/combowidget/triangle_up_light.svg")
};
QPair<QString,QString> arrowButtonDark = {
    QStringLiteral(":/res/combowidget/triangle_down_dark.svg"),
    QStringLiteral(":/res/combowidget/triangle_up_dark.svg")
};
}

ComboWidget::ComboWidget(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::ComboWidget)
    , popup(new PopupComboWidget(this))
{
    ui->setupUi(this);

    popup->setVisible(false);
    popup->setAnchorWidget(ui->inputFrame);

    themeNotifier = darkTheme_.addNotifier([this] {
        qDebug() << "darkTheme_ Notifier" << darkTheme_.value();
        CUR::StyleHelper::setTheme(this, darkTheme_.value());
        updateStyles();
    });
    darkTheme_.setValue(CUR::Theme::instance()->isDarkTheme());

    promptLabel = new QLabel(this);
    promptLabel->setObjectName(QStringLiteral("promptLabel"));
    promptLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    promptLabel->setMaximumHeight(16);
    promptLabel->setVisible(false);

    blockMouseTimer.setSingleShot(true);

    ui->arrowButton->setCursor(Qt::PointingHandCursor);


    connect(popup, &PopupComboWidget::clickedOutside, this, [this] {
        popup->hide();
        updateButtonIcon();
        //blockMouseTimer.start(QApplication::doubleClickInterval());
        blockMouseTimer.start(1000);

        auto selected = popup->selectedDevice();
        if (selected) {
            selectedDevice = selected;
            if (selectedDevice->friendlyName.isEmpty())
                setText(selectedDevice->certificateCommonName);
            else
                setText(selectedDevice->friendlyName);
        }
    });

    connect(ui->arrowButton, &QToolButton::clicked, this, [this] {
        popup->updateAndShow();
        updateButtonIcon();
    });

    // MacOS hover enable
    ui->arrowButton->setAttribute(Qt::WA_Hover, true);

    connect(ui->lineEdit, &QLineEdit::textChanged, this, &ComboWidget::onTextChanged);
    ui->lineEdit->setReadOnly(true);

    // Disable selections
    connect(ui->lineEdit, &QLineEdit::selectionChanged, this, [this] {
        ui->lineEdit->setSelection(0, 0);
    });

    setErrorState(false);
    updatePromptPosition();
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
    ui->lineEdit->setToolTip(val);
    ui->lineEdit->setSelection(0, 0);
}

void ComboWidget::setItems(const QList<Device> &list)
{
    QList<Device> tmpitems(list);
    qSwap(tmpitems, deviceList);

    popup->setItems(deviceList);

    // List is empty
    if (deviceList.isEmpty()) {
        selectedDevice = std::nullopt;
        setText(QStringLiteral(""));
        return;
    }

    // Update text if already selected
    if (selectedDevice) {
        const auto& dev = findByCN(deviceList, selectedDevice->certificateCommonName);
        if (dev) {
            if (dev->friendlyName.isEmpty())
                setText(dev->certificateCommonName);
            else
                setText(dev->friendlyName);
        }
    }

    // Still no selected
    if (text().isEmpty()) {
        selectedDevice = deviceList.first();
        if (selectedDevice->friendlyName.isEmpty())
            setText(selectedDevice->certificateCommonName);
        else
            setText(selectedDevice->friendlyName);
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

void ComboWidget::resizeEvent(QResizeEvent */*event*/)
{
    updatePromptPosition();
}

void ComboWidget::onTextChanged(const QString& str)
{
    if (!promptLabel->text().isEmpty()) {
        promptLabel->setVisible(!str.isEmpty());
    }

    emit textChanged(str);
}

void ComboWidget::updatePromptPosition()
{
    promptLabel->move(ui->lineEdit->pos().x() + 4, rect().top());
}

void ComboWidget::updateStyles()
{
    if (errorState)
        setStyleSheet(CUR::StyleHelper::loadFileToString(darkTheme_.value() ? inputStyleError.second : inputStyleError.first));
    else
        setStyleSheet(CUR::StyleHelper::loadFileToString(darkTheme_.value() ? inputStyle.second : inputStyle.first));

    updateButtonIcon();
}

void ComboWidget::updateButtonIcon()
{
    if (darkTheme_.value()) {
        ui->arrowButton->setIcon(popup->isVisible() ? QIcon(arrowButtonDark.second) : QIcon(arrowButtonDark.first));
    }
    else {
        ui->arrowButton->setIcon(popup->isVisible() ? QIcon(arrowButtonLight.second) : QIcon(arrowButtonLight.first));
    }
    update();
}

std::optional<Device> ComboWidget::findByCN(const QList<Device> &list, const QString& cn)
{
    const auto& it = std::find_if(list.cbegin(), list.cend(), [cn](const Device& dev) {
        return dev.certificateCommonName == cn;
    });

    if (it != list.cbegin())
        return *it;

    return std::nullopt;
}
