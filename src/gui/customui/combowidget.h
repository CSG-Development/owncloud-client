#pragma once

#include <QFrame>
#include <QTimer>
#include <QProperty>

#include "device/devicetypes.h"

class QLineEdit;
class QLabel;
class QToolButton;
class QComboBox;
class PopupComboWidget;

namespace Ui {class ComboWidget;}

class ComboWidget: public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool darkTheme READ isDarkTheme WRITE setDarkTheme BINDABLE bindableDarkTheme)

public:
    explicit ComboWidget(QWidget* parent = nullptr);
    ~ComboWidget();

    void setFontPixelSize(int val);
    void setPlaceholderText(const QString& str);

    QString text() const;
    void setText(const QString& val);

    void setItems(const QList<Device> &list);
    std::optional<Device> currentDevice() const {return selectedDevice;}

    void setErrorState(bool enable, const QString& txt = QString());

    bool isDarkTheme() const { return darkTheme_; }
    void setDarkTheme(bool v) { darkTheme_ = v; }
    QBindable<bool> bindableDarkTheme() {return &darkTheme_;}

    void setFavoriteDevice(const QString& deviceCN) {favoriteDeviceCN = deviceCN;}

Q_SIGNALS:
    void textChanged(const QString &);
    void textEdited(const QString &);
    void editingFinished();

    void buttonClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void onTextChanged(const QString&);
    void updatePromptPosition();
    void updateStyles();
    void updateButtonIcon();

    std::optional<Device> findByCN(const QList<Device> &list, const QString& cn);

private:
    Ui::ComboWidget* ui = nullptr;

    QLabel* promptLabel = nullptr;
    PopupComboWidget* popup = nullptr;

    bool errorState = false;
    QList<Device> deviceList;
    QTimer blockMouseTimer;
    std::optional<Device> selectedDevice;
    QString favoriteDeviceCN;

    QProperty<bool> darkTheme_ {false};
    QPropertyNotifier themeNotifier;
};
