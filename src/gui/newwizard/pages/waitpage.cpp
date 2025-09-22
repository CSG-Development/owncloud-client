#include "waitpage.h"
#include "ui_waitpage.h"
#include "gui/customui/stylehelper.h"
#include "theme.h"

namespace {
QPair<QString,QString> widgetStyle = {QStringLiteral(":/res/login/wait_page_light.qss"),QStringLiteral(":/res/login/wait_page_dark.qss")};
}

WaitPage::WaitPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WaitPage)
{
    ui->setupUi(this);

    updateTheme();
}

WaitPage::~WaitPage()
{
    delete ui;
}

void WaitPage::updateTheme()
{
    bool isDark = CUR::Theme::instance()->isDarkTheme();

    setStyleSheet(CUR::StyleHelper::loadFileToString(isDark ? widgetStyle.second : widgetStyle.first));

    for (auto widget: findChildren<QWidget*>()) {
        if (widget->metaObject()->indexOfMethod("setDarkTheme()") != -1)
            QMetaObject::invokeMethod(widget, "setDarkTheme");
    }
    update();
}
