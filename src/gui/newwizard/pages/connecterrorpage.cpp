#include "connecterrorpage.h"
#include "ui_connecterrorpage.h"
#include "gui/customui/stylehelper.h"
#include "theme.h"

namespace {
constexpr int fontSize = 16;
QPair<QString,QString> widgetStyle = {
    QStringLiteral(":/res/login/connect_error_page_light.qss"),
    QStringLiteral(":/res/login/connect_error_page_dark.qss")
};
QPair<QString,QString> backIcon = {
    QStringLiteral(":/res/login/arrow_back_btn_light.svg"),
    QStringLiteral(":/res/login/arrow_back_btn_dark.svg")
};
}

ConnectErrorPage::ConnectErrorPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ConnectErrorPage)
{
    ui->setupUi(this);

    connect(ui->btnBack, &QToolButton::clicked, this, &ConnectErrorPage::backClicked);
    connect(ui->btnRetry, &QToolButton::clicked, this, &ConnectErrorPage::retryClicked);

    ui->btnUnused->setEnabled(false);

    updateTheme();
}

ConnectErrorPage::~ConnectErrorPage()
{
    delete ui;
}

void ConnectErrorPage::updateTheme()
{
    bool isDark = APP::Theme::instance()->isDarkTheme();

    APP::StyleHelper::invoke_setDarkTheme_recursive(this);

    // QToolButton "icon" property does not supported in qss
    ui->btnBack->setIcon(isDark ? QIcon(backIcon.second) : QIcon(backIcon.first));
    setStyleSheet(APP::StyleHelper::loadFileToString(isDark ? widgetStyle.second : widgetStyle.first));

    update();
}
