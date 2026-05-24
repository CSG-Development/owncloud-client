#include "confirmdlgmac.h"
#include "ui_confirmdlgmac.h"
#include "platform/common/windowdragger.h"
#include "dlgutils.h"
#include "theme.h"
#include <utility>

namespace {
const std::pair<QString,QString> logo_icon = {
    QStringLiteral(":/res/Files-app-icon-round.svg"),
    QStringLiteral(":/res/Files-app-icon-round.svg")
};
const std::pair<QString,QString> warn_icon = {
    QStringLiteral(":/res/warn_mac_light.png"),
    QStringLiteral(":/res/warn_mac_dark.png")
};
const auto widget_style = QStringLiteral(":/platform/macos/confirmdlgmac.qss");
}

ConfirmDlgMac::ConfirmDlgMac(QWidget *parent)
    : BaseConfirmDlg(parent)
    , ui(new Ui::ConfirmDlgMac)
{
    ui->setupUi(this);
    DlgUtils::clearStyleSheet(this);
    setStyleSheet(DlgUtils::loadFileToString(widget_style));

    // Enable adjustSize() works properly (macos)
    if (layout())
        layout()->setSizeConstraint(QLayout::SetFixedSize);

    ui->btnAccept->setProperty("buttonStyle", CustomPushButtonStyle::Accent);
    ui->btnReject->setProperty("buttonStyle", CustomPushButtonStyle::Standard);

    CommonDialogWidgets widgets;
    widgets.lblHeader = ui->lblHeader;
    widgets.lblText = ui->lblText;
    widgets.btnAccept = ui->btnAccept;
    widgets.btnReject = ui->btnReject;
    widgets.btnIcon = ui->btnIcon;
    widgets.frame = ui->frame;
    widgets.frameHeader = ui->frameHeader;

    setupCommonLogic(widgets);

    new WindowDragger(ui->frameHeader, this);
}

ConfirmDlgMac::~ConfirmDlgMac()
{
    delete ui;
}

void ConfirmDlgMac::syncUI()
{
    bool isDark = APP::Theme::instance()->isDarkTheme();

    if (_ctrl->warningIconVisible.value()) {
        ui->btnIcon->setIcon(isDark ? QIcon(warn_icon.second) : QIcon(warn_icon.first));
    } else {
        ui->btnIcon->setIcon(isDark ? QIcon(logo_icon.second) : QIcon(logo_icon.first));
    }

    ui->frameButtons->setFixedHeight(_ctrl->singleButton ? 32 : 68);

    BaseConfirmDlg::syncUI();

    //_widgets.frame->updateGeometry();
    _widgets.lblHeader->updateGeometry();
    _widgets.lblText->updateGeometry();
    ui->frameText->updateGeometry();
    updateGeometry();

    if (layout())
        layout()->activate();

    // macOS can ignore adjustSize(). Here is trick to force it.
    QCoreApplication::processEvents();

    adjustSize();
}

void ConfirmDlgMac::applyTheme(bool isDark)
{
    ui->btnAccept->setProperty("darkMode", isDark);
    ui->btnReject->setProperty("darkMode", isDark);

    if (_ctrl->warningIconVisible.value()) {
        ui->btnIcon->setIcon(isDark ? QIcon(warn_icon.second) : QIcon(warn_icon.first));
    } else {
        ui->btnIcon->setIcon(isDark ? QIcon(logo_icon.second) : QIcon(logo_icon.first));
    }
}
