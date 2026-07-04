#include "onboardingpage5.h"

#include "ui_onboardingpage5.h"

OnboardingPage5::OnboardingPage5(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OnboardingPage5)
{
    ui->setupUi(this);
}

OnboardingPage5::~OnboardingPage5()
{
    delete ui;
}
