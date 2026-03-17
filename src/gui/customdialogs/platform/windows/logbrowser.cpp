#include "logbrowser.h"
#include "ui_logbrowser.h"
#include "platform/common/windowdragger.h"
#include "gui/customui/focusproxy.h"
#include "dlgutils.h"
#include "theme.h"

namespace {
const auto widget_style = QStringLiteral(":/platform/windows/logbrowser.qss");
const auto logo_icon = QStringLiteral(":/res/Files-logo.png");
const auto warn_icon = QStringLiteral(":/res/warn_sign.png");
const std::pair<QString,QString> close_icon = {
    QStringLiteral(":/res/close_light.svg"),
    QStringLiteral(":/res/close_dark.svg"),
};
const std::pair<FrameData,FrameData> frame_data = {
    {6, 2, QColor(0, 0, 0, 222), QColor(0, 0, 0, 0), 0},
    {6, 2, QColor(255, 255, 255, 222), QColor(0, 0, 0, 0), 0}
};
}

LogBrowser::LogBrowser(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LogBrowser)
{
    ui->setupUi(this);

    DlgUtils::setTransparent(this);
    DlgUtils::applyDropShadowDialog(ui->frame);

    DlgUtils::clearStyleSheet(this);
    setStyleSheet(DlgUtils::loadFileToString(widget_style));

    ui->btnOpenFolder->setStyle(new FocusProxyStyle(ui->btnOpenFolder));
    ui->btnClose->setStyle(new FocusProxyStyle(ui->btnClose));

    connect(ui->btnClose, &QPushButton::clicked, this, &LogBrowser::reject);
    connect(ui->btnHeadClose, &QToolButton::clicked, this, &LogBrowser::reject);
    connect(ui->btnOpenFolder, &QPushButton::clicked, this, &LogBrowser::openLocation);

    ui->btnIconTitle->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->btnIconTitle->setIcon(QIcon(logo_icon));
    ui->btnIcon->setIcon(QIcon(warn_icon));

    ui->btnClose->installEventFilter(this);
    ui->btnOpenFolder->installEventFilter(this);

    connect(ui->chkEnableLogging, &CCheckBox::checkStateChanged, this, [this](Qt::CheckState state) {
        emit logEnableChanged(state == Qt::Checked);
    });
    connect(ui->chkLogHttp, &CCheckBox::checkStateChanged, this, [this](Qt::CheckState state) {
        emit logHttpEnableChanged(state == Qt::Checked);
    });
    connect(ui->edFilesKeep, &QSpinBox::valueChanged, this, &LogBrowser::filesToKeepChanged);

    connect(APP::Theme::instance(), &APP::Theme::themeChanged, this, &LogBrowser::themeChanged);
    themeChanged();

    new WindowDragger(ui->frameTitle, this);
}

LogBrowser::~LogBrowser()
{
    delete ui;
}

void LogBrowser::setLocation(const QString &loc)
{
    _location = loc;
    ui->lblLocation->setText(tr("If enabled, logs will be written to: %1").arg(_location));
}

QString LogBrowser::location() const
{
    return _location;
}

void LogBrowser::setEnableLogging(bool enable)
{
    ui->chkEnableLogging->setChecked(enable);
}

bool LogBrowser::enableLogging() const
{
    return ui->chkEnableLogging->isChecked();
}

void LogBrowser::setEnableHttpLogging(bool enable)
{
    ui->chkLogHttp->setChecked(enable);
}

bool LogBrowser::enableHttpLogging() const
{
    return ui->chkLogHttp->isChecked();
}

void LogBrowser::setFilesToKeep(int count)
{
    ui->edFilesKeep->setValue(count);
}

int LogBrowser::filesToKeep() const
{
    return ui->edFilesKeep->value();
}

bool LogBrowser::eventFilter(QObject *watched, QEvent *event)
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

void LogBrowser::themeChanged()
{
    bool isDarkTheme = APP::Theme::instance()->isDarkTheme();

    ui->frameCloseBtn->setFrameData(isDarkTheme ? frame_data.second : frame_data.first);
    ui->frameOpenFolderBtn->setFrameData(isDarkTheme ? frame_data.second : frame_data.first);

    DlgUtils::setTheme(this, isDarkTheme);
    ui->btnHeadClose->setIcon(isDarkTheme ? QIcon(close_icon.second) : QIcon(close_icon.first));
}
