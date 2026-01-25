#pragma once

#include "device/devicetypes.h"
#include "account.h"

#include <QWidget>
#include <QAbstractTableModel>

namespace Ui { class DevWidget; }

class DevModel: public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns {
        clUrl = 0,
        clPort,
        clType,
        clOrigin,
        clOobeDone,
        clCount
    };

    enum TabRole {
        IdRole = Qt::UserRole + 1
    };


    explicit DevModel(QWidget *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void setDeviceData(const QList<DevicePath>& d);

private:
    QList<DevicePath> data_;
};


class DevWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DevWidget(QWidget *parent = nullptr);
    ~DevWidget();

    void setDevice(const Device& dev);
    void setAccout(CUR::Account* acc);

    void onCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

    void updatePathInfo();

private:
    void showPathInfoText();
    void beginTable();
    void endTable();
    void insertData(const QString &key, const QString &value);


private:
    Ui::DevWidget *ui = nullptr;
    DevModel model_;
    CUR::Account* acc_ = nullptr;
    QUuid currentId_;
    Device device_;
    QString _htmlBuffer;
};
