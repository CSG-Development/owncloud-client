#include "onboardingpage4.h"

#include "ui_onboardingpage4.h"

OnboardingPage4::OnboardingPage4(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OnboardingPage4)
{
    ui->setupUi(this);
}

OnboardingPage4::~OnboardingPage4()
{
    delete ui;
}
