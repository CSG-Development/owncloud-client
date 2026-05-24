#include "confirmdlg.h"
#include "dlgutils.h"
#include "platform/common/confirmdlgcontroller.h"
#include "platform/common/windowdragger.h"
#include "platform/windows/ui_confirmdlg.h"
#include "theme.h"
#include "ui_confirmdlg.h"

namespace {
const auto normal_width = 448;
const auto wide_width = 620;
const auto widget_style = QStringLiteral(":/platform/windows/confirmdlg.qss");
const auto logo_icon = QStringLiteral(":/res/Files-app-icon-round.svg");
const auto warning_icon = QStringLiteral(":/res/warning_1_export.svg");
const std::pair<QString,QString> close_icon = {
    QStringLiteral(":/res/close_light.svg"),
    QStringLiteral(":/res/close_dark.svg"),
};
const std::pair<FrameData,FrameData> frame_data = {
    {6, 2, QColor(0, 0, 0, 222), QColor(0, 0, 0, 0), 0},
    {6, 2, QColor(255, 255, 255, 222), QColor(0, 0, 0, 0), 0}
};

}

ConfirmDlg::ConfirmDlg(QWidget *parent)
    : BaseConfirmDlg(parent)
    , ui(new Ui::ConfirmDlg)
{
    ui->setupUi(this);
    DlgUtils::clearStyleSheet(this);
    setStyleSheet(DlgUtils::loadFileToString(widget_style));

    ui->frameAcceptBtn->setFrameData(APP::Theme::instance()->isDarkTheme() ? frame_data.second : frame_data.first);
    ui->frameRejectBtn->setFrameData(APP::Theme::instance()->isDarkTheme() ? frame_data.second : frame_data.first);

    CommonDialogWidgets widgets;
    widgets.lblHeader = ui->lblHeader;
    widgets.lblText = ui->lblText;
    widgets.btnAccept = ui->btnAccept;
    widgets.btnReject = ui->btnReject;
    widgets.btnIcon = ui->btnIcon;
    widgets.btnIconWarn = ui->btnIconWarn;
    widgets.frame = ui->frame;
    widgets.frameHeader = ui->frameHeader;
    widgets.frameAcceptBtn = ui->frameAcceptBtn;
    widgets.frameRejectBtn = ui->frameRejectBtn;

    setupCommonLogic(widgets);

    ui->btnClose->setIcon(APP::Theme::instance()->isDarkTheme() ? QIcon(close_icon.second) : QIcon(close_icon.first));
    connect(ui->btnClose, &QToolButton::clicked, this, &ConfirmDlg::reject);

    ui->btnIcon->setIcon(QIcon(logo_icon));
    ui->btnIconWarn->setIcon(QIcon(warning_icon));

    new WindowDragger(ui->frameHeader, this);
    syncUI();
}

ConfirmDlg::~ConfirmDlg()
{
    delete ui;
}

void ConfirmDlg::syncUI()
{
    BaseConfirmDlg::syncUI();

    _widgets.btnIconWarn->setVisible(_ctrl->warningIconVisible);
    ui->frameIconWarn->setVisible(_ctrl->warningIconVisible.value());
    if (_ctrl->singleButton) {
        ui->buttonSpacer->changeSize(1, 1, QSizePolicy::Expanding);
    }
    else {
        ui->buttonSpacer->changeSize(0, 0, QSizePolicy::Ignored);
    }

    int width = _ctrl->isWide ? wide_width : normal_width;
    // _widgets.frame->setMinimumWidth(width);
    _widgets.frame->setFixedWidth(width);

    _widgets.frame->updateGeometry();
    _widgets.lblHeader->updateGeometry();
    _widgets.lblText->updateGeometry();
    if (layout())
        layout()->activate();
    adjustSize();
}

void ConfirmDlg::applyTheme(bool isDark)
{
    ui->frameAcceptBtn->setFrameData(isDark ? frame_data.second : frame_data.first);
    ui->frameRejectBtn->setFrameData(isDark ? frame_data.second : frame_data.first);
}
