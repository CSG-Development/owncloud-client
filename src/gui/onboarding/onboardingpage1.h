#pragma once

#include <QWidget>

namespace Ui
{
class OnboardingPage1;
}

class OnboardingPage1 : public QWidget
{
    Q_OBJECT

public:
    explicit OnboardingPage1(QWidget *parent = nullptr);
    ~OnboardingPage1();

private:
    Ui::OnboardingPage1 *ui = nullptr;
};
