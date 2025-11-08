#pragma once

#include <QWidget>

namespace Ui { class ConnectErrorPage; }

class ConnectErrorPage : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectErrorPage(QWidget *parent = nullptr);
    ~ConnectErrorPage();

    void updateTheme();

Q_SIGNALS:
    void backClicked();
    void retryClicked();

private:
    Ui::ConnectErrorPage *ui = nullptr;
};
