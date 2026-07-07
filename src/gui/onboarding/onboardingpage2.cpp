#include "onboardingpage2.h"

#include "ui_onboardingpage2.h"

OnboardingPage2::OnboardingPage2(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OnboardingPage2)
{
    ui->setupUi(this);
}

OnboardingPage2::~OnboardingPage2()
{
    delete ui;
}
