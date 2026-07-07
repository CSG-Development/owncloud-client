#include "onboardingpage3.h"

#include "ui_onboardingpage3.h"

OnboardingPage3::OnboardingPage3(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OnboardingPage3)
{
    ui->setupUi(this);
}

OnboardingPage3::~OnboardingPage3()
{
    delete ui;
}
