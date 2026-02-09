#pragma once

#include <QPushButton>
#include <QProperty>

class LoginPushButton: public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool darkTheme READ isDarkTheme WRITE setDarkTheme BINDABLE bindableDarkTheme)

public:

    enum class IconSidePosition {
        Left,
        Right
    };

    explicit LoginPushButton(QWidget* parent = nullptr);

    void setIconSidePosition(IconSidePosition side) {side_ = side; update();}
    void setSideIcon(const QIcon& icon) {icon_ = icon; update();}

    bool isDarkTheme() const { return darkTheme_.value(); }
    void setDarkTheme(bool v) { darkTheme_.setValue(v); }

    QBindable<bool> bindableDarkTheme() {return &darkTheme_;}

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    IconSidePosition side_ = IconSidePosition::Left;
    QIcon icon_;
    QPropertyNotifier themeNotifier;
    QProperty<bool> darkTheme_ {false};
};
