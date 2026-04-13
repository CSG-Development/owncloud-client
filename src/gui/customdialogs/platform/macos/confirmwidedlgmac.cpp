#include "confirmwidedlgmac.h"
#include "ui_confirmwidedlgmac.h"
#include "platform/common/windowdragger.h"
#include "dlgutils.h"
#include "theme.h"
#include <utility>

namespace {
const std::pair<QString,QString> logo_icon = {
    QStringLiteral(":/res/logo_mac_light.png"),
    QStringLiteral(":/res/logo_mac_dark.png")
};
const std::pair<QString,QString> warn_icon = {
    QStringLiteral(":/res/warn_mac_light.png"),
    QStringLiteral(":/res/warn_mac_dark.png")
};
const auto widget_style = QStringLiteral(":/platform/macos/confirmwidedlgmac.qss");
}

ConfirmWideDlgMac::ConfirmWideDlgMac(QWidget *parent)
    : BaseConfirmDlg(parent)
    , ui(new Ui::ConfirmWideDlgMac)
{
    ui->setupUi(this);
    DlgUtils::clearStyleSheet(this);
    setStyleSheet(DlgUtils::loadFileToString(widget_style));

    bool isDark = APP::Theme::instance()->isDarkTheme();
    applyTheme(isDark);
    ui->btnIcon->setIcon(isDark ? QIcon(warn_icon.second) : QIcon(warn_icon.first));

    CommonDialogWidgets widgets;
    widgets.lblHeader = ui->lblHeader;
    widgets.lblText = ui->lblText;
    widgets.btnAccept = ui->btnAccept;
    widgets.btnReject = ui->btnReject;
    widgets.btnIcon = ui->btnIcon;
    widgets.frame = ui->frame;
    widgets.frameHeader = ui->frameHeader;
    widgets.frameIcon = ui->frameIcon;

    setupCommonLogic(widgets);

    new WindowDragger(ui->frameHeader, this);
    syncUI();
}

ConfirmWideDlgMac::~ConfirmWideDlgMac()
{
    delete ui;
}

void ConfirmWideDlgMac::syncUI()
{
    BaseConfirmDlg::syncUI();

    _widgets.frameIcon->setVisible(_ctrl->warningIconVisible);

    _widgets.frame->updateGeometry();
    _widgets.lblHeader->updateGeometry();
    _widgets.lblText->updateGeometry();
    if (layout()) {
        layout()->invalidate();
        layout()->activate();
    }
    QTimer::singleShot(0, this, [this]() {
        this->adjustSize();
    });
}

void ConfirmWideDlgMac::applyTheme(bool isDark)
{
    if (_ctrl->warningIconVisible.value()) {
        ui->btnIcon->setIcon(isDark ? QIcon(warn_icon.second) : QIcon(warn_icon.first));
    }
    // else {
    //     ui->btnIcon->setIcon(isDark ? QIcon(logo_icon.second) : QIcon(logo_icon.first));
    // }
}
