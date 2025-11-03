#include "popupcombomodel.h"

PopupComboModel::PopupComboModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int PopupComboModel::rowCount(const QModelIndex &/*parent*/) const
{
    return data_.size();
}

QVariant PopupComboModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return {};

    int row = idx.row();
    int col = idx.column();

    switch (role)
    {
    case Qt::DisplayRole:
        if (col == clName) {
            return data_[row].certificateCommonName;
        }
        break;

    // case Qt::ToolTipRole:
    //     if (col == clName) {
    //         return data_[row].;
    //     }
    //     break;

    case DeviceInfoRole:
        return QVariant::fromValue(data_[row]);
    }

    return {};
}

void PopupComboModel::setDeviceInfoList(const QList<Device> &list)
{
    QList<Device> tmp(list);
    beginResetModel();
    qSwap(data_, tmp);
    endResetModel();
}
