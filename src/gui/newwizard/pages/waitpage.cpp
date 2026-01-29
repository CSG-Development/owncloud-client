#include "waitpage.h"
#include "ui_waitpage.h"
#include "gui/customui/stylehelper.h"
#include "theme.h"

WaitPage::WaitPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WaitPage)
{
    ui->setupUi(this);

    setStyleSheet(CUR::StyleHelper::loadFileToString(QStringLiteral(":/res/login/wait_page.qss")));
    themeNotifier = darkTheme_.addNotifier([this] {
        CUR::StyleHelper::setTheme(this, darkTheme_.value());
    });
    darkTheme_.setValue(CUR::Theme::instance()->isDarkTheme());
}

WaitPage::~WaitPage()
{
    delete ui;
}

