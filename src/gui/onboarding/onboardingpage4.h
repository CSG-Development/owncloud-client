#pragma once

#include <QWidget>

namespace Ui
{
class OnboardingPage4;
}

class OnboardingPage4 : public QWidget
{
    Q_OBJECT

public:
    explicit OnboardingPage4(QWidget *parent = nullptr);
    ~OnboardingPage4();

private:
    Ui::OnboardingPage4 *ui = nullptr;
};
