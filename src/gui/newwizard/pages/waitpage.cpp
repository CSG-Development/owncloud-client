#include "waitpage.h"
#include "ui_waitpage.h"
#include "gui/customui/stylehelper.h"
#include "theme.h"

namespace {
const QString logo_image = QStringLiteral(":/res/Files-app-icon-gradient.svg");
const std::pair<QString,QString> header_icon = {
    QStringLiteral(":/res/login_logo_light.svg"),
    QStringLiteral(":/res/login_logo_dark.svg")
};
}

WaitPage::WaitPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WaitPage)
{
    ui->setupUi(this);

    setStyleSheet(APP::StyleHelper::loadFileToString(QStringLiteral(":/res/login/wait_page.qss")));

    ui->btnImage->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->btnLogo->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->btnImage->setIcon(QIcon(logo_image));

    themeNotifier = darkTheme_.addNotifier([this] {
        updateTheme();
    });

    darkTheme_.setValue(APP::Theme::instance()->isDarkTheme());
    updateTheme();
}

WaitPage::~WaitPage()
{
    delete ui;
}

void WaitPage::updateTheme()
{
    APP::StyleHelper::setTheme(this, darkTheme_.value());
    ui->btnLogo->setIcon(darkTheme_.value() ? QIcon(header_icon.second) : QIcon(header_icon.first));
}
