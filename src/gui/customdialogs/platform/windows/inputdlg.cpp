#include "inputdlg.h"
#include "ui_inputdlg.h"
#include "dlgutils.h"
#include "platform/common/windowdragger.h"
#include "theme.h"

namespace {
const auto widget_style = QStringLiteral(":/platform/windows/inputdlg.qss");
const auto logo_icon = QStringLiteral(":/res/Files-app-icon-round.svg");
const std::pair<QString,QString> close_icon = {
    QStringLiteral(":/res/close_light.svg"),
    QStringLiteral(":/res/close_dark.svg")
};

const std::pair<FrameData,FrameData> frame_data = {
    {6, 2, QColor(0, 0, 0, 222), QColor(0, 0, 0, 0), 0},
    {6, 2, QColor(255, 255, 255, 222), QColor(0, 0, 0, 0), 0}
};

}

InputDlg::InputDlg(QWidget *parent)
    : BaseInputDlg(parent)
    , ui(new Ui::InputDlg)
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
    widgets.frame = ui->frame;
    widgets.frameHeader = ui->frameHeader;
    widgets.frameAcceptBtn = ui->frameAcceptBtn;
    widgets.frameRejectBtn = ui->frameRejectBtn;
    widgets.edText = ui->edText;

    setupCommonLogic(widgets);

    ui->btnIcon->setIcon(QIcon(logo_icon));
    ui->btnClose->setIcon(APP::Theme::instance()->isDarkTheme() ? QIcon(close_icon.second) : QIcon(close_icon.first));
    connect(ui->btnClose, &QToolButton::clicked, this, &InputDlg::reject);

    new WindowDragger(ui->frameHeader, this);
}

InputDlg::~InputDlg()
{
    delete ui;
}

void InputDlg::syncUI()
{
    BaseInputDlg::syncUI();
    _widgets.frame->updateGeometry();
    ui->frameText->updateGeometry();
    if (layout())
        layout()->activate();
    adjustSize();
}

void InputDlg::applyTheme(bool isDark)
{
    ui->frameAcceptBtn->setFrameData(isDark ? frame_data.second : frame_data.first);
    ui->frameRejectBtn->setFrameData(isDark ? frame_data.second : frame_data.first);
}
