#pragma once

#include <QWidget>

namespace Ui
{
class OnboardingPage3;
}

class OnboardingPage3 : public QWidget
{
    Q_OBJECT

public:
    explicit OnboardingPage3(QWidget *parent = nullptr);
    ~OnboardingPage3();

private:
    Ui::OnboardingPage3 *ui = nullptr;
};
