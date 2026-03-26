#include "finishedpage.h"
#include "ui_finishedpage.h"
#include "gui/customui/stylehelper.h"
#include "gui/customui/loginpushbutton.h"
#include "gui/customui/radioindicatorproxy.h"
#include "gui/customdialogs/custommessagebox.h"
#include "gui/customdialogs/dlgutils.h"
#include "theme.h"

#include <QFileDialog>
#include <QMouseEvent>

namespace {
constexpr int smallHeight = 232;
constexpr int advHeight = 540;
constexpr QSize icon_size = {24, 24};
const auto widget_style = QStringLiteral(":/res/login/finished_page.qss");
std::pair<QString,QString> doneIcon = {
    QStringLiteral(":/res/login/done_light.svg"),
    QStringLiteral(":/res/login/done_dark.svg")
};
std::pair<QString,QString> backIcon = {
    QStringLiteral(":/res/login/back_arrow_light.svg"),
    QStringLiteral(":/res/login/back_arrow_dark.svg")
};
std::pair<QString,QString> browseIcon = {
    QStringLiteral(":/res/login/browse_light.svg"),
    QStringLiteral(":/res/login/browse_dark.svg")
};
std::pair<QString,QString> resetIcon = {
    QStringLiteral(":/res/login/reset_light.svg"),
    QStringLiteral(":/res/login/reset_dark.svg")
};
std::pair<QString,QString> warnIcon = {
    QStringLiteral(":/res/login/vfs_warning_light.svg"),
    QStringLiteral(":/res/login/vfs_warning_dark.svg")
};
const std::pair<FrameData,FrameData> frame_data = {
    {6, 2, QColor(0, 0, 0, 222), QColor(0, 0, 0, 0), 0},
    {6, 2, QColor(255, 255, 255, 222), QColor(0, 0, 0, 0), 0}
};

}

FinishedPage::FinishedPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FinishedPage)
{
    ui->setupUi(this);

    setStyleSheet(APP::StyleHelper::loadFileToString(widget_style));

    ui->btnBrowse->setIconSize(icon_size);
    ui->btnReset->setIconSize(icon_size);

    ui->rbDownloadEverything->setStyle(new RadioIndicatorProxy(ui->rbDownloadEverything));
    ui->rbConfigManually->setStyle(new RadioIndicatorProxy(ui->rbConfigManually));
    ui->rbUseVfs->setStyle(new RadioIndicatorProxy(ui->rbUseVfs));

    themeNotifier = darkTheme_.addNotifier([this] {
        updateTheme();
    });
    darkTheme_.setValue(APP::Theme::instance()->isDarkTheme());

    connect(ui->chkAdvanced, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState state) {
        advancedStateChanged(state == Qt::Checked);
    });
    advancedStateChanged(false);

    connect(ui->btnBrowse, &QToolButton::clicked, this, &FinishedPage::onBrowseClicked);
    connect(ui->btnBack, &QPushButton::clicked, this, &FinishedPage::backClicked);
    connect(ui->btnDone, &QPushButton::clicked, this, [this] {
        Q_EMIT doneClicked(syncMode(), syncTargetDir());
    });

    ui->btnReset->setEnabled(false);
    connect(ui->edDownloadDir, &InputWidget::textChanged, this, [this] {
        ui->btnReset->setEnabled(QDir::toNativeSeparators(ui->edDownloadDir->text()) != QDir::toNativeSeparators(defaultTargetDir_));
    });
    connect(ui->btnReset, &QToolButton::clicked, this, [this] {
        ui->edDownloadDir->setText(QDir::toNativeSeparators(defaultTargetDir_));
    });

    ui->btnDone->setIconSidePosition(LoginPushButton::IconSidePosition::Right);

    ui->btnVfsIcon->setAttribute(Qt::WA_TransparentForMouseEvents);

    ui->lblUseVfs->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->lblConfigManually->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->lblDownloadEverything->setAttribute(Qt::WA_TransparentForMouseEvents);

    ui->frameDownloadEverything->setFrameData(APP::Theme::instance()->isDarkTheme() ? frame_data.second : frame_data.first);
    ui->frameConfigManually->setFrameData(APP::Theme::instance()->isDarkTheme() ? frame_data.second : frame_data.first);
    ui->frameFocusUseVfs->setFrameData(APP::Theme::instance()->isDarkTheme() ? frame_data.second : frame_data.first);

    ui->frameFocusUseVfs->setFocusPolicy(Qt::StrongFocus);
    ui->frameFocusUseVfs->installEventFilter(this);

    ui->frameConfigManually->setFocusPolicy(Qt::StrongFocus);
    ui->frameConfigManually->installEventFilter(this);

    ui->frameDownloadEverything->setFocusPolicy(Qt::StrongFocus);
    ui->frameDownloadEverything->installEventFilter(this);

    // Handle focus frame
    ui->rbDownloadEverything->installEventFilter(this);
    ui->rbConfigManually->installEventFilter(this);
    ui->rbUseVfs->installEventFilter(this);

    auto radioHandler = [this](QRadioButton* sender, QList<QRadioButton*> others) {
        return [this, sender, others](bool toggled) {
            const QSignalBlocker _b1(ui->rbDownloadEverything),
                _b2(ui->rbConfigManually),
                _b3(ui->rbUseVfs);
            if (toggled)
                for (auto* rb : others) rb->setChecked(false);
            else
                sender->setChecked(true);
        };
    };

    connect(ui->rbDownloadEverything, &QRadioButton::toggled, this, radioHandler(ui->rbDownloadEverything, {ui->rbConfigManually, ui->rbUseVfs}));
    connect(ui->rbConfigManually, &QRadioButton::toggled, this, radioHandler(ui->rbConfigManually, {ui->rbDownloadEverything, ui->rbUseVfs}));
    connect(ui->rbUseVfs, &QRadioButton::toggled, this, radioHandler(ui->rbUseVfs, {ui->rbDownloadEverything, ui->rbConfigManually}));


    updateTheme();
}

FinishedPage::~FinishedPage()
{
    delete ui;
}

void FinishedPage::setupPageDefaults(const QString &defaultSyncTargetDir, const QString &userChosenSyncTargetDir,
                                     bool vfsIsAvailable, bool enableVfsByDefault, bool vfsModeIsExperimental)
{
    {
        const QSignalBlocker b(ui->edDownloadDir);
        ui->edDownloadDir->setText(QDir::toNativeSeparators(userChosenSyncTargetDir));
    }
    ui->rbDownloadEverything->setChecked(true);

    // could also make it invisible, but then the UX is different for different installations
    // this may be overwritten by a branding option (see below)
    // ui->rbUseVfs->setEnabled(vfsIsAvailable);
    // ui->rbUseVfs->setText(tr("Use &virtual files instead of downloading content immediately"));
    ui->frameFocusUseVfs->setEnabled(vfsIsAvailable);

    if (vfsModeIsExperimental) {
        // ui->rbUseVfs->setIcon(QIcon(QStringLiteral(":/res/login/warning_light.svg")));

        // when a feature is experimental and experimental features are disabled globally, it should be hidden
        if (!APP::Theme::instance()->enableExperimentalFeatures()) {
            ui->rbUseVfs->hide();
        }
    }

    if (!APP::Theme::instance()->showVirtualFilesOption()) {
        ui->rbUseVfs->setVisible(false);
        enableVfsByDefault = false;
    }

    if (!vfsIsAvailable) {
        enableVfsByDefault = false;
    }

    if (!vfsIsAvailable) {
        // fallback: it's set as default option in Qt Designer, but we should make sure the option is selected if VFS is not available
        ui->rbDownloadEverything->setChecked(true);
        ui->rbUseVfs->setToolTip(tr("The virtual filesystem feature is not available for this installation."));
    } else if (vfsModeIsExperimental) {
        ui->rbUseVfs->setToolTip(tr("The virtual filesystem feature is not stable yet. Use with caution."));
    }

    // this should be handled on application startup, too
    if (APP::Theme::instance()->forceVirtualFilesOption()) {
        if (!vfsModeIsExperimental) {
            // note: this might fail when the VFS plugins have not been built (yet) as well
            Q_ASSERT(vfsIsAvailable);
        }
    }

    // vfsIsAvailable is false when experimental features are not enabled and the mode is experimental even if a plugin is found
    if (vfsIsAvailable && APP::Theme::instance()->forceVirtualFilesOption()) {
        // this has no visual effect, but is needed for syncMode()
        ui->rbUseVfs->setChecked(true);

        // we want to hide the entire sync mode selection from the user, not just disable it
        ui->chkAdvanced->setVisible(false);
    }

#if 1
    if (vfsModeIsExperimental) {
        connect(ui->rbUseVfs, &QRadioButton::clicked, this, [this]() {
            auto messageBox = new APP::CustomMessageBox(this);
            messageBox->setHeaderText(tr("Enable experimental feature?"))
                .setMessageText(tr("When the \"virtual files\" mode is enabled no files will be downloaded initially. "
                   "Instead, a tiny file will be created for each file that exists on the server. "
                   "The contents can be downloaded by running these files or by using their context menu."
                   "\n\n"
                   "The virtual files mode is mutually exclusive with selective sync. "
                   "Currently unselected folders will be translated to online-only folders "
                   "and your selective sync settings will be reset."
                   "\n\n"
                   "Switching to this mode will abort any currently running synchronization."
                   "\n\n"
                   "This is a new, experimental mode. If you decide to use it, please report any "
                   "issues that come up."))
                .setAcceptButtonText(tr("Enable experimental placeholder mode"))
                .setRejectButtonText(tr("Stay safe"))
                .setWide(true)
                .setWarningIconVisible(true)
                .setDeleteOnClose(true);

            connect(messageBox, &APP::CustomMessageBox::rejected, this, [this]() {
                // bring back to "safety"
                ui->rbDownloadEverything->setChecked(true);
            });

#ifdef Q_OS_MACOS
            messageBox->show();
#else
            messageBox->open();
#endif
        });
    }
#endif

    defaultTargetDir_ = defaultSyncTargetDir;
    //ui->edDownloadDir->setEnabled(ui->edDownloadDir->text() != QDir::toNativeSeparators(defaultTargetDir_));
    ui->btnDone->setFocus();
}

void FinishedPage::updateTheme()
{
    ui->btnDone->setSideIcon(QIcon(darkTheme_.value() ? doneIcon.second : doneIcon.first));
    ui->btnBack->setSideIcon(QIcon(darkTheme_.value() ? backIcon.second : backIcon.first));
    APP::StyleHelper::setTheme(this, darkTheme_.value());

    ui->btnBrowse->setIcon(QIcon(darkTheme_.value() ? browseIcon.second : browseIcon.first));
    ui->btnReset->setIcon(QIcon(darkTheme_.value() ? resetIcon.second : resetIcon.first));
    ui->btnVfsIcon->setIcon(QIcon(darkTheme_.value() ? warnIcon.second : warnIcon.first));

    update();
}

APP::Wizard::SyncMode FinishedPage::syncMode() const
{
    if (ui->rbDownloadEverything->isChecked()) {
        return APP::Wizard::SyncMode::SyncEverything;
    }
    if (ui->rbConfigManually->isChecked()) {
        return APP::Wizard::SyncMode::ConfigureUsingFolderWizard;
    }
    if (ui->rbUseVfs->isChecked()) {
        return APP::Wizard::SyncMode::UseVfs;
    }
    return APP::Wizard::SyncMode::SyncEverything;
}

QString FinishedPage::syncTargetDir() const
{
    return QDir::toNativeSeparators(ui->edDownloadDir->text());
}

void FinishedPage::handleFrameEvent(QEvent *event, QRadioButton* button)
{
    if (event->type() == QEvent::Enter) {
        button->setAttribute(Qt::WA_UnderMouse, true);
        button->update();
    } else if (event->type() == QEvent::Leave) {
        button->setAttribute(Qt::WA_UnderMouse, false);
        button->update();
    }
}

bool FinishedPage::handleFrameMouse(QEvent *event, QRadioButton* button)
{
    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonDblClick) {
        auto *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            button->setFocus(Qt::MouseFocusReason);
            button->setDown(true);
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        auto *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            button->setDown(false);
            button->click();
            return true;
        }
    }
    return false;
}

void FinishedPage::handleFocusEvent(QEvent *event, FocusFrame* frame)
{
    if (event->type() == QEvent::FocusIn) {
        frame->setProperty("focused", true);
    }
    else if (event->type() == QEvent::FocusOut) {
        frame->setProperty("focused", false);
    } else {
        return;
    }
    frame->update();
}

bool FinishedPage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->frameFocusUseVfs) {
        handleFrameEvent(event, ui->rbUseVfs);
        handleFrameMouse(event, ui->rbUseVfs);
    }
    else if (watched == ui->frameConfigManually) {
        handleFrameEvent(event, ui->rbConfigManually);
        handleFrameMouse(event, ui->rbConfigManually);
    }
    else if (watched == ui->frameDownloadEverything) {
        handleFrameEvent(event, ui->rbDownloadEverything);
        handleFrameMouse(event, ui->rbDownloadEverything);
    }
    else if (watched == ui->rbUseVfs) {
        handleFocusEvent(event, ui->frameFocusUseVfs);
    }
    else if (watched == ui->rbDownloadEverything) {
        handleFocusEvent(event, ui->frameDownloadEverything);
    }
    else if (watched == ui->rbConfigManually) {
        handleFocusEvent(event, ui->frameConfigManually);
    }

    return QWidget::eventFilter(watched, event);
}

void FinishedPage::advancedStateChanged(bool checked)
{
    ui->frameAdvancedContext->setVisible(checked);
    ui->frameContent->setMinimumHeight(checked ? advHeight : smallHeight);
    ui->frameContent->setMaximumHeight(checked ? 1000 : smallHeight);
    update();
}

void FinishedPage::onBrowseClicked()
{
    auto dialog = new QFileDialog(this, tr("Select the local folder"), ui->edDownloadDir->text());
    dialog->setFileMode(QFileDialog::Directory);
    dialog->setOption(QFileDialog::ShowDirsOnly);

    connect(dialog, &QFileDialog::fileSelected, this, [this](const QString &directory) {
        // the directory chooser should guarantee that the directory exists
        Q_ASSERT(QDir(directory).exists());

        ui->edDownloadDir->setText(QDir::toNativeSeparators(directory));
    });
    dialog->open();
}

