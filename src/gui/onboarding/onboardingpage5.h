#pragma once

#include <QWidget>

namespace Ui
{
class OnboardingPage5;
}

class OnboardingPage5 : public QWidget
{
    Q_OBJECT

public:
    explicit OnboardingPage5(QWidget *parent = nullptr);
    ~OnboardingPage5();

private:
    Ui::OnboardingPage5 *ui;
};
