#pragma once

#include "platform/common/pushbuttonstyle.h"

#include <QPushButton>
#include <QPainter>
#include <QEnterEvent>

class CustomPushButtonMac : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool darkMode READ isDarkMode WRITE setDarkMode)
    Q_PROPERTY(CustomPushButtonStyle buttonStyle READ buttonStyle WRITE setButtonStyle)

public:
    explicit CustomPushButtonMac(QWidget *parent = nullptr);

    void setButtonStyle(CustomPushButtonStyle style);
    CustomPushButtonStyle buttonStyle() const { return _buttonStyle;}

    void setDarkMode(bool dark);
    bool isDarkMode() const { return _darkMode; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    CustomPushButtonStyle _buttonStyle = CustomPushButtonStyle::Accent;
    bool _darkMode = false;
};
