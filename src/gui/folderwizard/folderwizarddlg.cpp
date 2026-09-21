#include "folderwizarddlg.h"

#include "ui_folderwizarddlg.h"

#include "gui/customdialogs/dlgutils.h"
#include "gui/customdialogs/platform/common/windowdragger.h"
#include "gui/customui/stylehelper.h"
#include "theme.h"

#include <QIcon>
#include <QPushButton>
#include <QToolButton>

static void initFolderWizardDlgResources()
{
    Q_INIT_RESOURCE(folderwizard_res);
    Q_INIT_RESOURCE(customdialogs_res);
}

namespace APP {

namespace
{
const auto windowsStyle = QStringLiteral(":/platform/windows/folderwizarddlg.qss");
const auto macosStyle = QStringLiteral(":/platform/macos/folderwizarddlg.qss");
const auto logoIcon = QStringLiteral(":/res/Files-app-icon-round.svg");
const std::pair<QString, QString> closeIcon = {
    QStringLiteral(":/res/close_light.svg"),
    QStringLiteral(":/res/close_dark.svg"),
};

QString folderWizardStylePath()
{
#ifdef Q_OS_WINDOWS
    return windowsStyle;
#else
    return macosStyle;
#endif
}
}

FolderWizardDlg::FolderWizardDlg(const AccountStatePtr &account, QWidget *parent)
    : QDialog(parent)
    , ui(new ::Ui::FolderWizardDlg)
    , _wizard(new FolderWizard(account, this))
{
    static bool resourcesLoaded = []() {
        initFolderWizardDlgResources();
        return true;
    }();

    ui->setupUi(this);
    StyleHelper::applyPushButtonsStyle(this);

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);
    setWindowTitle(FolderWizard::tr("Add folder sync connection"));
    ui->lblHeaderTitle->setText(windowTitle());
    ui->btnIconTitle->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->lblHeaderTitle->setAttribute(Qt::WA_TransparentForMouseEvents);

#ifdef Q_OS_WINDOWS
    DlgUtils::setTransparent(this);
    DlgUtils::applyDropShadowDialog(ui->frameRoot);
    ui->btnIconTitle->setIcon(QIcon(logoIcon));
    new WindowDragger(ui->frameTitle, this);
#else
    layout()->setContentsMargins(0, 0, 0, 0);
    ui->frameTitle->setVisible(false);
#endif

    ui->verticalLayoutContent->addWidget(_wizard);

    ui->btnCancel->setText(tr("Cancel"));
    ui->btnBack->setText(tr("< Back"));

    connect(_wizard, &FolderWizard::completed, this, &QDialog::accept);
    connect(_wizard, &FolderWizard::navigationChanged, this, &FolderWizardDlg::updateNavigation);
    connect(ui->btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->btnClose, &QToolButton::clicked, this, &QDialog::reject);
    connect(ui->btnBack, &QPushButton::clicked, _wizard, &FolderWizard::back);
    connect(ui->btnNext, &QPushButton::clicked, _wizard, &FolderWizard::next);
    connect(Theme::instance(), &Theme::themeChanged, this, &FolderWizardDlg::updateTheme);

    updateTheme(Theme::instance()->isDarkTheme());
    updateNavigation();
}

FolderWizardDlg::~FolderWizardDlg()
{
    delete ui;
}

FolderWizard::Result FolderWizardDlg::result()
{
    return _wizard->result();
}

void FolderWizardDlg::present()
{
    show();
    raise();
    activateWindow();
}

void FolderWizardDlg::updateTheme(bool isDark)
{
    StyleHelper::applyThemedStyleSheet(this, folderWizardStylePath(), isDark);
    ui->btnClose->setIcon(QIcon(isDark ? closeIcon.second : closeIcon.first));
}

void FolderWizardDlg::updateNavigation()
{
    ui->btnBack->setEnabled(_wizard->canGoBack());
    ui->btnNext->setEnabled(_wizard->canGoNext());
    ui->btnNext->setText(_wizard->isLastPage() ? FolderWizard::tr("Add Sync Connection") : tr("Next >"));
}

} // namespace APP
