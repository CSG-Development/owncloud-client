#include "popupcombomodel.h"

PopupComboModel::PopupComboModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int PopupComboModel::rowCount(const QModelIndex &/*parent*/) const
{
    return data_.devices().size();
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
            if (data_.devices()[row].friendlyName.isEmpty())
                return data_.devices()[row].certificateCommonName;
            else
                return data_.devices()[row].friendlyName;
        }
        break;

    // case Qt::ToolTipRole:
    //     if (col == clName) {
    //         return data_[row].;
    //     }
    //     break;

    case DeviceInfoRole:
        return QVariant::fromValue(data_.devices()[row]);
    }

    return {};
}

void PopupComboModel::setDeviceInfoList(const DeviceList& list)
{
    DeviceList tmp(list);
    beginResetModel();
    qSwap(data_, tmp);
    endResetModel();
}
