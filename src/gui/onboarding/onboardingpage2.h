#pragma once

#include <QWidget>

namespace Ui
{
class OnboardingPage2;
}

class OnboardingPage2 : public QWidget
{
    Q_OBJECT

public:
    explicit OnboardingPage2(QWidget *parent = nullptr);
    ~OnboardingPage2();

private:
    Ui::OnboardingPage2 *ui = nullptr;
};
