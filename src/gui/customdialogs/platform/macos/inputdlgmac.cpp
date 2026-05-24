#include "inputdlgmac.h"
#include "ui_inputdlgmac.h"
#include "dlgutils.h"
#include "platform/common/windowdragger.h"
#include "theme.h"

namespace {
const auto widget_style = QStringLiteral(":/platform/macos/inputdlgmac.qss");
const auto logo_icon = QStringLiteral(":/res/Files-app-icon-round.svg");
const std::pair<QString,QString> close_icon = {
    QStringLiteral(":/res/close_light.svg"),
    QStringLiteral(":/res/close_dark.svg")
};

const std::pair<FrameData,FrameData> frame_data = {
    {6, 4, QColor(25, 118, 210, 127), QColor(0, 0, 0, 0), 0},
    {6, 4, QColor(222, 222, 222, 255), QColor(0, 0, 0, 0), 0}
};

const std::pair<QString,QString> clear_icon = {
    QStringLiteral(":/res/clear_button_light.svg"),
    QStringLiteral(":/res/clear_button_dark.svg")
};

}

InputDlgMac::InputDlgMac(QWidget *parent)
    : BaseInputDlg(parent)
    , ui(new Ui::InputDlgMac)
{
    ui->setupUi(this);
    DlgUtils::clearStyleSheet(this);
    setStyleSheet(DlgUtils::loadFileToString(widget_style));

    bool isDark = APP::Theme::instance()->isDarkTheme();
    clearAction = new QAction(isDark ? QIcon(clear_icon.second) : QIcon(clear_icon.first), tr("Clear"));

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
    ui->btnClose->setIcon(isDark ? QIcon(close_icon.second) : QIcon(close_icon.first));
    connect(ui->btnClose, &QToolButton::clicked, this, &InputDlgMac::reject);

    ui->edText->addAction(clearAction, QLineEdit::TrailingPosition);
    connect(clearAction, &QAction::triggered, this, [this] {
        ui->edText->clear();
    });

    ui->btnClose->setVisible(false);
    ui->btnIcon->setVisible(false);
    ui->frameIconAligner->setVisible(false);

    new WindowDragger(ui->frameHeader, this);
}

InputDlgMac::~InputDlgMac()
{
    delete ui;
}

void InputDlgMac::syncUI()
{
    BaseInputDlg::syncUI();
    _widgets.frame->updateGeometry();
    ui->frameText->updateGeometry();
    if (layout())
        layout()->activate();
    adjustSize();
}

void InputDlgMac::applyTheme(bool isDark)
{
    clearAction->setIcon(isDark ? QIcon(clear_icon.second) : QIcon(clear_icon.first));
}
