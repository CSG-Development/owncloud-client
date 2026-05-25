#pragma once

#include <QWidget>
#include <QProperty>

namespace Ui {class WaitPage;}

class WaitPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool darkTheme READ isDarkTheme WRITE setDarkTheme BINDABLE bindableDarkTheme)

public:
    explicit WaitPage(QWidget *parent = nullptr);
    ~WaitPage();

    bool isDarkTheme() const { return darkTheme_.value(); }
    void setDarkTheme(bool v) { darkTheme_.setValue(v); }

    QBindable<bool> bindableDarkTheme() {return &darkTheme_;}

    void updateTheme();

private:
    Ui::WaitPage* ui = nullptr;
    QPropertyNotifier themeNotifier;
    QProperty<bool> darkTheme_ {false};
};
