#include "devwidget.h"
#include "device/deviceapi.h"
#include "ui_devwidget.h"

#include "device/networkmonitor.h"

#include <QScrollBar>

namespace {
const auto BtnStyle = QStringLiteral(
    "#btnRefresh, #btnNetw, #btnQueryAbout, #btnSave {"
    "    border: 1px solid rgba(0, 0, 40, 0.4);"
    "    background-color: transparent;"
    "    color: #000040;"
    "    padding: 2px 8px;"
    "    border-radius: 10px;"
    "    font-size: 13px;"
    "}"
    "#btnRefresh:hover, #btnNetw:hover, #btnQueryAbout:hover, #btnSave:hover {"
    "    background-color: rgba(0, 0, 238, 0.08);"
    "}"
    "#btnRefresh:pressed, #btnNetw:pressed, #btnQueryAbout:pressed, #btnSave:pressed {"
    "    background-color: rgba(0, 0, 238, 0.20);"
    "}"
    "#btnApply {"
    "    border: 1px solid rgba(200, 0, 0, 0.4);"
    "    background-color: rgba(200, 0, 0, 0.03);"
    "    padding: 2px 8px;"
    "    border-radius: 10px;"
    "    font-size: 13px;"
    "}"
    "#btnApply:hover {"
    "    background-color: rgba(200, 0, 0, 0.08);"
    "}"
    "#btnApply:pressed {"
    "    background-color: rgba(200, 0, 0, 0.2);"
    "}"
    );
}

DevWidget::DevWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DevWidget)
{
    ui->setupUi(this);

    setStyleSheet(BtnStyle);

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);

    ui->cbDeviceType->addItem(tr("Unknown"), QVariant::fromValue(DeviceType::Unknown));
    ui->cbDeviceType->addItem(tr("Local"), QVariant::fromValue(DeviceType::Local));
    ui->cbDeviceType->addItem(tr("Public"), QVariant::fromValue(DeviceType::Public));
    ui->cbDeviceType->addItem(tr("Remote"), QVariant::fromValue(DeviceType::Remote));

    ui->cbOrigin->addItem(tr("Unknown"), QVariant::fromValue(DeviceOrigin::Unknown));
    ui->cbOrigin->addItem(tr("Remote"), QVariant::fromValue(DeviceOrigin::Remote));
    ui->cbOrigin->addItem(tr("mDNS"), QVariant::fromValue(DeviceOrigin::MDNS));
    ui->cbOrigin->addItem(tr("Static"), QVariant::fromValue(DeviceOrigin::Static));

    ui->tableView->setModel(&model_);
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &DevWidget::onCurrentChanged);

    connect(ui->btnRefresh, &QPushButton::clicked, this, [this] {
        setDevice(acc_->device());
    });
    connect(ui->btnApply, &QPushButton::clicked, this, [this] {
        acc_->setDevice(device_);
    });
    connect(ui->btnSave, &QPushButton::clicked, this, [this] {
        if (!currentId_.isNull()) {
            auto path = device_.getPathPtr(currentId_);
            if (path) {
                path->address = ui->edAddress->text().trimmed();
                path->port = ui->edPort->value();
                path->deviceType = static_cast<DeviceType>(ui->cbDeviceType->currentData().toInt());
                path->origin = static_cast<DeviceOrigin>(ui->cbOrigin->currentData().toInt());
                model_.setDeviceData(device_.paths);
            }
        }
    });
    connect(ui->btnNetw, &QPushButton::clicked, this, [] {
        APP::NetworkMonitor::instance()->emit_network_changed();
    });
    connect(ui->btnQueryAbout, &QPushButton::clicked, this, [this] {
        auto path = device_.getPathPtr(currentId_);
        if (!path)
            return;

        auto dev_api = new DeviceApi(this);

        dev_api->query_about_status(DevHelpers::makeServerUrl(path->address, path->port, false, true))
            .then(this, [dev_api,path,this](const std::pair<AboutCtx,StatusCtx>& ctx) {
                path->about = ctx.first.deviceAbout;
                path->status = ctx.second.deviceStatus;
                dev_api->deleteLater();
                showPathInfoText();
            });

    });
    ui->btnQueryAbout->setEnabled(false);
}

DevWidget::~DevWidget()
{
    delete ui;
}

void DevWidget::setDevice(const Device &dev)
{
    device_ = dev;
    ui->lblCommonName->setText(dev.certificateCommonName.isEmpty() ? QStringLiteral("---") : dev.certificateCommonName);
    ui->lblFriendlyName->setText(dev.friendlyName.isEmpty() ? QStringLiteral("---") : dev.friendlyName);
    ui->lblHostname->setText(dev.hostname.isEmpty() ? QStringLiteral("---") : dev.hostname);
    ui->lblDevId->setText(dev.seagateDeviceID.isEmpty() ? QStringLiteral("---") : dev.seagateDeviceID);
    ui->lblOrigin->setText(DevHelpers::originToStr(dev.origin));

    model_.setDeviceData(dev.paths);
    showPathInfoText();
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
            case clType: return DevHelpers::devTypeToStr(data_[row].deviceType);
            case clOrigin: return DevHelpers::originToStr(data_[row].origin);
            case clOobeDone: return (data_[row].status.oobe_done ? tr("true") : tr("false"));
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
        case clOobeDone: return tr("OOBE Done");
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

void DevWidget::setAccout(APP::Account *acc)
{
    acc_ = acc;
    setDevice(acc_->device());
}

void DevWidget::onCurrentChanged(const QModelIndex &current, const QModelIndex &/*previous*/)
{
    ui->btnQueryAbout->setEnabled(false);
    if (!current.isValid()) {
        currentId_ = QUuid();
        return;
    }

    currentId_ = current.data(DevModel::IdRole).toUuid();
    // qDebug() << currentId_;
    updatePathInfo();
    showPathInfoText();
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
        ui->edAddress->setText(QStringLiteral("---"));
        ui->edPort->setValue(0);
        ui->cbDeviceType->setCurrentIndex(0);
        ui->cbOrigin->setCurrentIndex(0);
    }
}

void DevWidget::showPathInfoText()
{
    ui->textBrowser->clear();
    _htmlBuffer.clear();
    if (currentId_.isNull()) {
        ui->textBrowser->append(QStringLiteral("Select device path"));
        return;
    }
    else {
        if (auto path = device_.findPath(currentId_)) {
            beginTable();
            insertData(QStringLiteral("<b>Status</b>"), QStringLiteral(""));
            insertData(QStringLiteral("app_files"), path->status.app_files);
            insertData(QStringLiteral("app_photos"), path->status.app_photos);
            insertData(QStringLiteral("oobe_done"), path->status.oobe_done ? QStringLiteral("true") : QStringLiteral("false"));
            insertData(QStringLiteral("state"), path->status.state);

            insertData(QStringLiteral(""), QStringLiteral(""));
            insertData(QStringLiteral("<b>About</b>"), QStringLiteral(""));
            insertData(QStringLiteral("certificate_CN"), path->about.certificate_common_name);
            insertData(QStringLiteral("date"), path->about.date.toString());
            insertData(QStringLiteral("default_mac_addr"), path->about.default_mac_addr);
            insertData(QStringLiteral("hostname"), path->about.hostname);
            insertData(QStringLiteral("install_id"), path->about.install_id);
            insertData(QStringLiteral("model_name"), path->about.model_name);
            insertData(QStringLiteral("model_number"), path->about.model_number);
            insertData(QStringLiteral("os_state"), path->about.os_state);
            insertData(QStringLiteral("product_id"), path->about.product_id);
            insertData(QStringLiteral("serial_number"), path->about.serial_number);
            insertData(QStringLiteral("version"), path->about.version);

            insertData(QStringLiteral("memory"), QString::number(path->about.hardware_info.memory));
            insertData(QStringLiteral("processor_count"), QString::number(path->about.hardware_info.processor_count));
            insertData(QStringLiteral("processor_type"), path->about.hardware_info.processor_type);

            for (const auto& it : std::as_const(path->about.network_interfaces)) {
                insertData(QStringLiteral("Interface"), QStringLiteral("IP: %1, Mask: %2, Gateway: %3")
                                                            .arg(it.ipv4_info.ipv4)
                                                            .arg(it.ipv4_info.netmask)
                                                            .arg(it.ipv4_info.gateway));
            }

            endTable();
            ui->btnQueryAbout->setEnabled(true);
        }
        else {
            ui->textBrowser->append(QStringLiteral("Error get current device path"));
            return;
        }
    }
    ui->textBrowser->append(_htmlBuffer);
    ui->textBrowser->verticalScrollBar()->setValue(0);
}

void DevWidget::beginTable()
{
    _htmlBuffer = QStringLiteral("<table style='border-collapse: collapse; width: auto;'>");
}

void DevWidget::endTable()
{
    _htmlBuffer += QStringLiteral("</table><br>");
}

void DevWidget::insertData(const QString &key, const QString &value)
{
    _htmlBuffer += QStringLiteral("<tr>"
                                  "<td style='padding-right: 20px; font-weight: bold; white-space: nowrap;'>%1</td>"
                                  "<td>%2</td></tr>").arg(key, value);
}
