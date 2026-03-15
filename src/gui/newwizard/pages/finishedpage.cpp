#include "finishedpage.h"
#include "ui_finishedpage.h"
#include "gui/customui/stylehelper.h"
#include "gui/customui/loginpushbutton.h"
#include "gui/customdialogs/custommessagebox.h"
#include "theme.h"

#include <QFileDialog>

namespace {
constexpr int smallHeight = 232;
constexpr int advHeight = 540;
QPair<QString,QString> doneIcon = {
    QStringLiteral(":/res/login/done_light.svg"),
    QStringLiteral(":/res/login/done_dark.svg")
};
QPair<QString,QString> backIcon = {
    QStringLiteral(":/res/login/back_arrow_light.svg"),
    QStringLiteral(":/res/login/back_arrow_dark.svg")
};
QPair<QString,QString> browseIcon = {
    QStringLiteral(":/res/login/browse_light.svg"),
    QStringLiteral(":/res/login/browse_dark.svg")
};
QPair<QString,QString> resetIcon = {
    QStringLiteral(":/res/login/reset_light.svg"),
    QStringLiteral(":/res/login/reset_dark.svg")
};
}

FinishedPage::FinishedPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FinishedPage)
{
    ui->setupUi(this);

    setStyleSheet(APP::StyleHelper::loadFileToString(QStringLiteral(":/res/login/finished_page.qss")));

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

    connect(ui->edDownloadDir, &InputWidget::textChanged, this, [this] {
        ui->btnReset->setEnabled(ui->edDownloadDir->text() != QDir::toNativeSeparators(defaultTargetDir_));
    });
    connect(ui->btnReset, &QToolButton::clicked, this, [this] {
        ui->edDownloadDir->setText(QDir::toNativeSeparators(defaultTargetDir_));
    });

    ui->btnDone->setIconSidePosition(LoginPushButton::IconSidePosition::Right);
    showErrorMessage({});
}

FinishedPage::~FinishedPage()
{
    delete ui;
}

void FinishedPage::setupPageDefaults(const QString &defaultSyncTargetDir, const QString &userChosenSyncTargetDir, bool vfsIsAvailable, bool enableVfsByDefault, bool vfsModeIsExperimental)
{
    ui->edDownloadDir->setText(QDir::toNativeSeparators(userChosenSyncTargetDir));
    ui->rbDownloadEverything->setChecked(true);

    // could also make it invisible, but then the UX is different for different installations
    // this may be overwritten by a branding option (see below)
    ui->rbUseVfs->setEnabled(vfsIsAvailable);
    ui->rbUseVfs->setText(tr("Use &virtual files instead of downloading content immediately"));

    if (vfsModeIsExperimental) {
        ui->rbUseVfs->setIcon(QIcon(QStringLiteral(":/res/login/warning_light.svg")));

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

    defaultTargetDir_ = defaultSyncTargetDir;
    ui->edDownloadDir->setEnabled(ui->edDownloadDir->text() != QDir::toNativeSeparators(defaultTargetDir_));
}

void FinishedPage::updateTheme()
{
    ui->btnDone->setSideIcon(QIcon(darkTheme_.value() ? doneIcon.second : doneIcon.first));
    ui->btnBack->setSideIcon(QIcon(darkTheme_.value() ? backIcon.second : backIcon.first));
    APP::StyleHelper::setTheme(this, darkTheme_.value());
    update();
}

void FinishedPage::showErrorMessage(const QString &msg)
{
    ui->frameErrorMessage->setVisible(!msg.isEmpty());
    ui->lblErrorText->setText(msg);
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

void FinishedPage::advancedStateChanged(bool checked)
{
    ui->frameAdvanced->setVisible(checked);
    ui->frameContent->setMinimumHeight(checked ? advHeight : smallHeight);
    // ui->frameContent->setMaximumHeight(checked ? advHeight : smallHeight);
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

