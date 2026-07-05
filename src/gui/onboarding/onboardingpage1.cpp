#include "onboardingpage1.h"

#include "ui_onboardingpage1.h"

OnboardingPage1::OnboardingPage1(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OnboardingPage1)
{
    ui->setupUi(this);
}

OnboardingPage1::~OnboardingPage1()
{
    delete ui;
}
