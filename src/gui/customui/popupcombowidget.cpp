#include "popupcombowidget.h"
#include "ui_popupcombowidget.h"
#include "stylehelper.h"

#include <QMouseEvent>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QListWidget>
#include <QGraphicsDropShadowEffect>

namespace {

constexpr int blurRadius = 4;

QPair<QString,QString> widgetStyle = {
    QStringLiteral(":/res/combowidget/popup_combo_light.qss"),
    QStringLiteral(":/res/combowidget/popup_combo_dark.qss")
};

}

PopupComboWidget::PopupComboWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PopupComboWidget)
{
    ui->setupUi(this);
    setObjectName(QStringLiteral("popupComboWidget"));

    setWindowFlags(Qt::Popup|Qt::FramelessWindowHint|Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
    setMouseTracking(true);

    setDarkTheme(false);

    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(blurRadius);
    effect->setXOffset(3);
    effect->setYOffset(3);
    ui->frame->setGraphicsEffect(effect);

    ui->tableView->setModel(&model_);

    auto f = ui->tableView->font();
    f.setPixelSize(16);
    ui->tableView->setFont(f);

    ui->tableView->viewport()->installEventFilter(this);

    connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentChanged, this, [&](const QModelIndex &current, const QModelIndex &/*previous*/) {
        if (!current.isValid()) {
            currentItem = std::nullopt;
        }
        auto v = current.data(PopupComboModel::DeviceInfoRole);
        currentItem = v.value<Device>();
    });
}

PopupComboWidget::~PopupComboWidget()
{
    delete ui;
}

void PopupComboWidget::setAnchorWidget(QWidget* w)
{
    Q_ASSERT(w);
    anchor_ = w;
}

void PopupComboWidget::updateAndShow()
{
    auto x = anchor_->mapToGlobal(anchor_->rect().bottomLeft()).x();
    auto y = anchor_->mapToGlobal(anchor_->rect().bottomLeft()).y();
    move(x, y);
    resize(anchor_->rect().width(), 200);
    show();
}

void PopupComboWidget::setItems(const QList<Device> &list)
{
    model_.setDeviceInfoList(list);
}

bool PopupComboWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->tableView->viewport()) {
        if (currentItem) {
            if (event->type() == QEvent::MouseButtonRelease) {
                QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
                if (mouseEvent->button() == Qt::LeftButton)
                {
                    emit clickedOutside();
                    return true;
                }
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void PopupComboWidget::setDarkTheme(bool dark)
{
    setStyleSheet(CUR::StyleHelper::loadFileToString(dark ? widgetStyle.second : widgetStyle.first));
    style()->unpolish(this);
    style()->polish(this);
}

void PopupComboWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !rect().contains(event->position().toPoint()))
    {
        emit clickedOutside();
        return;
    }

    QWidget::mousePressEvent(event);
}
