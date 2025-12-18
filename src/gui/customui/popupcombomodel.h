#pragma once

#include "device/devicetypes.h"
#include <QAbstractTableModel>

class PopupComboModel: public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns {
        clName = 0,
        clCount
    };
    enum DeviceRole {
        DeviceInfoRole = Qt::UserRole + 1
    };

    explicit PopupComboModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex&) const override {return clCount;}
    QVariant data(const QModelIndex &idx, int role) const override;

    void setDeviceInfoList(const QList<Device>& list);

private:
    QList<Device> data_;
};
