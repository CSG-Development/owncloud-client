#include "logbrowsermac.h"
#include "ui_logbrowsermac.h"
#include "gui/customui/stylehelper.h"
#include "platform/common/windowdragger.h"
#include "dlgutils.h"
#include "theme.h"
#include "configfile.h"

namespace {
const auto widget_style = QStringLiteral(":/platform/macos/logbrowsermac.qss");
const auto logo_icon = QStringLiteral(":/res/Files-app-icon-round.svg");
const auto warn_icon = QStringLiteral(":/res/warn_sign.png");
const std::pair<FrameData,FrameData> frame_data = {
    {6, 2, QColor(0, 0, 0, 222), QColor(0, 0, 0, 0), 0},
    {6, 2, QColor(0, 0, 0, 222), QColor(0, 0, 0, 0), 0}
};
}

LogBrowserMac::LogBrowserMac(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LogBrowserMac)
{
    ui->setupUi(this);

    DlgUtils::setTransparent(this);
    DlgUtils::applyDropShadowDialog(ui->frame);

    DlgUtils::clearStyleSheet(this);
    setStyleSheet(DlgUtils::loadFileToString(widget_style));

    connect(ui->btnClose, &QPushButton::clicked, this, &LogBrowserMac::reject);
    connect(ui->btnOpenFolder, &QPushButton::clicked, this, &LogBrowserMac::openLocation);

    ui->btnIcon->setIcon(QIcon(warn_icon));

    ui->btnClose->installEventFilter(this);
    ui->btnOpenFolder->installEventFilter(this);

    connect(ui->chkEnableLogging, &CCheckBox::checkStateChanged, this, [this](Qt::CheckState state) {
        emit logEnableChanged(state == Qt::Checked);
    });
    connect(ui->chkLogHttp, &CCheckBox::checkStateChanged, this, [this](Qt::CheckState state) {
        emit logHttpEnableChanged(state == Qt::Checked);
    });
    connect(ui->edFilesKeep, &QSpinBox::valueChanged, this, &LogBrowserMac::filesToKeepChanged);

    connect(APP::Theme::instance(), &APP::Theme::themeChanged, this, &LogBrowserMac::themeChanged);
    themeChanged();

    new WindowDragger(ui->frameTitle, this);
}

LogBrowserMac::~LogBrowserMac()
{
    delete ui;
}

void LogBrowserMac::setLocation(const QString &loc)
{
    _location = loc;
    ui->lblLocation->setText(tr("If enabled, logs will be written to: %1").arg(_location));
}

QString LogBrowserMac::location() const
{
    return _location;
}

void LogBrowserMac::setEnableLogging(bool enable)
{
    ui->chkEnableLogging->setChecked(enable);
}

bool LogBrowserMac::enableLogging() const
{
    return ui->chkEnableLogging->isChecked();
}

void LogBrowserMac::setEnableHttpLogging(bool enable)
{
    ui->chkLogHttp->setChecked(enable);
}

bool LogBrowserMac::enableHttpLogging() const
{
    return ui->chkLogHttp->isChecked();
}

void LogBrowserMac::setFilesToKeep(int count)
{
    ui->edFilesKeep->setValue(count);
}

int LogBrowserMac::filesToKeep() const
{
    return ui->edFilesKeep->value();
}

bool LogBrowserMac::eventFilter(QObject *watched, QEvent *event)
{
    bool need_update = false;
    if (watched == ui->btnClose) {
        if (event->type() == QEvent::FocusIn) {
            ui->frameCloseBtn->setProperty("focused", true);
        }
        else if (event->type() == QEvent::FocusOut) {
            ui->frameCloseBtn->setProperty("focused", false);
        }
        ui->frameCloseBtn->style()->unpolish(ui->frameCloseBtn);
        ui->frameCloseBtn->style()->polish(ui->frameCloseBtn);
        need_update = true;
    }
    else if (watched == ui->btnOpenFolder) {
        if (event->type() == QEvent::FocusIn) {
            ui->frameOpenFolderBtn->setProperty("focused", true);
        }
        else if (event->type() == QEvent::FocusOut) {
            ui->frameOpenFolderBtn->setProperty("focused", false);
        }
        ui->frameOpenFolderBtn->style()->unpolish(ui->frameOpenFolderBtn);
        ui->frameOpenFolderBtn->style()->polish(ui->frameOpenFolderBtn);
        need_update = true;
    }
    if (need_update)
        update();

    return QDialog::eventFilter(watched, event);
}

void LogBrowserMac::themeChanged()
{
    bool isDarkTheme = APP::Theme::instance()->isDarkTheme();

    ui->frameCloseBtn->setFrameData(isDarkTheme ? frame_data.second : frame_data.first);
    ui->frameOpenFolderBtn->setFrameData(isDarkTheme ? frame_data.second : frame_data.first);

    DlgUtils::setTheme(this, isDarkTheme);
    APP::StyleHelper::invoke_setDarkTheme_recursive(this);
}
