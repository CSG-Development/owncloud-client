#pragma once

#include <QWidget>

namespace Ui {class WaitPage;}

class WaitPage : public QWidget
{
    Q_OBJECT

public:
    explicit WaitPage(QWidget *parent = nullptr);
    ~WaitPage();

    void updateTheme();

private:
    Ui::WaitPage* ui = nullptr;
};
