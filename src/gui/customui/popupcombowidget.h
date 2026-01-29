#pragma once

#include "device/devicetypes.h"
#include "popupcombomodel.h"

#include <QWidget>
#include <QProperty>

class QLineEdit;
class QListWidget;
class QFrame;

namespace Ui {class PopupComboWidget;}

class PopupComboWidget: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool darkTheme READ isDarkTheme WRITE setDarkTheme BINDABLE bindableDarkTheme)

public:
    explicit PopupComboWidget(QWidget* parent = nullptr);
    ~PopupComboWidget();

    void setAnchorWidget(QWidget* w);
    void updateAndShow();
    
    void setItems(const QList<Device> &list);
    std::optional<Device> selectedDevice() const {return currentItem;}

    bool eventFilter(QObject *watched, QEvent *event) override;

    bool isDarkTheme() const { return darkTheme_; }
    void setDarkTheme(bool v) { darkTheme_ = v; }
    QBindable<bool> bindableDarkTheme() {return &darkTheme_;}

signals:
    void clickedOutside();

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    Ui::PopupComboWidget* ui = nullptr;

    QWidget* anchor_ = nullptr;
    std::optional<Device> currentItem;
    PopupComboModel model_;

    QProperty<bool> darkTheme_ {false};
    QPropertyNotifier themeNotifier;
};
