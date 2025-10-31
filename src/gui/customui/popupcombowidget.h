#pragma once

#include "gui/newwizard/loginservices/devicetypes.h"
#include "popupcombomodel.h"
#include <QWidget>

class QLineEdit;
class QListWidget;
class QFrame;

namespace Ui {class PopupComboWidget;}

class PopupComboWidget: public QWidget
{
    Q_OBJECT

public:
    explicit PopupComboWidget(QWidget* parent = nullptr);
    ~PopupComboWidget();

    void setAnchorWidget(QWidget* w);
    void updateAndShow();
    
    void setItems(const QList<DeviceInfo> &list);
    std::optional<DeviceInfo> selectedDevice() const {return currentItem;}

    bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    void setDarkTheme(bool dark);

signals:
    void clickedOutside();

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    Ui::PopupComboWidget* ui = nullptr;

    QWidget* anchor_ = nullptr;
    std::optional<DeviceInfo> currentItem;
    PopupComboModel model_;
};
