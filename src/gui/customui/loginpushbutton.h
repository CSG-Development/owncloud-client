#pragma once

#include <QPushButton>

class LoginPushButton: public QPushButton
{
    Q_OBJECT

public:

    enum class IconSidePosition {
        Left,
        Right
    };

    explicit LoginPushButton(QWidget* parent = nullptr);

    void setIconSidePosition(IconSidePosition side) {side_ = side; update();}
    void setSideIcon(const QIcon& icon) {icon_ = icon; update();}

    Q_INVOKABLE void setDarkTheme();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    IconSidePosition side_ = IconSidePosition::Left;
    QIcon icon_;
    bool isDark = false;
};
