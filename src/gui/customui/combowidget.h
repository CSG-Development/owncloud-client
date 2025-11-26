#pragma once

#include <QFrame>
#include <QTimer>

#include "devicetypes.h"

class QLineEdit;
class QLabel;
class QToolButton;
class QComboBox;
class PopupComboWidget;

namespace Ui {class ComboWidget;}

class ComboWidget: public QFrame
{
    Q_OBJECT

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

    bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    void setDarkTheme();

Q_SIGNALS:
    void textChanged(const QString &);
    void textEdited(const QString &);
    void editingFinished();

    void buttonClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void onTextChanged(const QString&);
    void updatePromptPosition();
    void updateStyles();
    void updateButtonIcon();

private:
    Ui::ComboWidget* ui = nullptr;

    QLabel* promptLabel = nullptr;
    PopupComboWidget* popup = nullptr;

    bool isDark = false;
    bool errorState = false;
    QList<Device> deviceList;
    QTimer blockMouseTimer;
    std::optional<Device> selectedDevice;
};
