#include "devwidget.h"
#include "ui_devwidget.h"

#include "networkjobs/networkmonitor.h"

DevWidget::DevWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DevWidget)
{
    ui->setupUi(this);

    ui->cbDeviceType->addItem(tr("Unknown"), QVariant::fromValue(DeviceType::Unknown));
    ui->cbDeviceType->addItem(tr("Local"), QVariant::fromValue(DeviceType::Local));
    ui->cbDeviceType->addItem(tr("Public"), QVariant::fromValue(DeviceType::Public));
    ui->cbDeviceType->addItem(tr("Remote"), QVariant::fromValue(DeviceType::Remote));

    ui->cbOrigin->addItem(tr("Unknown"), QVariant::fromValue(DevicePathOrigin::Unknown));
    ui->cbOrigin->addItem(tr("Remote"), QVariant::fromValue(DevicePathOrigin::Remote));
    ui->cbOrigin->addItem(tr("mDNS"), QVariant::fromValue(DevicePathOrigin::MDNS));
    ui->cbOrigin->addItem(tr("Static"), QVariant::fromValue(DevicePathOrigin::Static));

    ui->tableView->setModel(&model_);
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &DevWidget::onCurrentChanged);

    connect(ui->btnRefresh, &QPushButton::clicked, this, [&] {
        setDevice(acc_->device());
    });
    connect(ui->btnApply, &QPushButton::clicked, this, [&] {
        acc_->setDevice(device_);
    });
    connect(ui->btnSave, &QPushButton::clicked, this, [&] {
        if (!currentId_.isNull()) {
            auto path = device_.getPathPtr(currentId_);
            if (path) {
                path->address = ui->edAddress->text().trimmed();
                path->port = ui->edPort->value();
                path->deviceType = static_cast<DeviceType>(ui->cbDeviceType->currentData().toInt());
                path->origin = static_cast<DevicePathOrigin>(ui->cbOrigin->currentData().toInt());
                model_.setDeviceData(device_.paths);
            }
        }
    });
    connect(ui->btnNetw, &QPushButton::clicked, this, [&] {
        CUR::NetworkMonitor::instance()->emit_network_changed();
    });
}

DevWidget::~DevWidget()
{
    delete ui;
}

void DevWidget::setDevice(const Device &dev)
{
    device_ = dev;
    ui->lblCommonName->setText(dev.certificateCommonName);
    ui->lblFriendlyName->setText(dev.friendlyName());
    ui->lblHostname->setText(dev.hostname);
    ui->lblDevId->setText(dev.seagateDeviceID);

    model_.setDeviceData(dev.paths);
}

DevModel::DevModel(QWidget *parent)
    : QAbstractTableModel(parent)
{
}

int DevModel::rowCount(const QModelIndex &/*parent*/) const
{
    return data_.size();
}

int DevModel::columnCount(const QModelIndex &/*parent*/) const
{
    return clCount;
}

QVariant DevModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return {};

    int row = idx.row();
    int col = idx.column();

    switch (role)
    {
    case Qt::DisplayRole:
        switch (col) {
            case clUrl: return data_[row].address;
            case clPort: return data_[row].port;
            case clType: return DevicePath::devTypeToStr(data_[row].deviceType);
            case clOrigin: return DevicePath::originToStr(data_[row].origin);
            case clOnline: return (data_[row].isOnline ? tr("Yes") : tr("No"));
        }
        break;

    case IdRole:
        return data_[row].id;
    }

    return {};

}

QVariant DevModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return {};

    if (role == Qt::DisplayRole)
    {
        switch (section)
        {
        case clUrl: return tr("Url");
        case clPort: return tr("Port");
        case clType: return tr("Type");
        case clOrigin: return tr("Origin");
        case clOnline: return tr("Online");
        }
    }

    return {};
}

void DevModel::setDeviceData(const QList<DevicePath> &d)
{
    beginResetModel();
    data_ = d;
    endResetModel();
}

void DevWidget::setAccout(CUR::Account *acc)
{
    acc_ = acc;
    setDevice(acc_->device());
}

void DevWidget::onCurrentChanged(const QModelIndex &current, const QModelIndex &/*previous*/)
{
    if (!current.isValid()) {
        currentId_ = QUuid();
        return;
    }

    currentId_ = current.data(DevModel::IdRole).toUuid();
    qDebug() << currentId_;
    updatePathInfo();
}

void DevWidget::updatePathInfo()
{
    auto path = device_.findPath(currentId_);
    if (path) {
        ui->edAddress->setText(path->address);
        ui->edPort->setValue(path->port);

        int idx = ui->cbDeviceType->findData(static_cast<int>(path->deviceType));
        ui->cbDeviceType->setCurrentIndex(idx);
        idx = ui->cbOrigin->findData(static_cast<int>(path->origin));
        ui->cbOrigin->setCurrentIndex(idx);
    }
    else {
        ui->edAddress->setText(tr("---"));
        ui->edPort->setValue(0);
        ui->cbDeviceType->setCurrentIndex(0);
        ui->cbOrigin->setCurrentIndex(0);
    }
}
