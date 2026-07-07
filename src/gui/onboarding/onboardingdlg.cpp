#include "onboardingdlg.h"

#include "onboardingpage1.h"
#include "onboardingpage2.h"
#include "onboardingpage3.h"
#include "onboardingpage4.h"
#include "onboardingpage5.h"

#include "ui_onboardingdlg.h"

#include "gui/customdialogs/dlgutils.h"
#include "theme.h"

#include <QApplication>
#include <QCloseEvent>
#include <QEvent>
#include <QMouseEvent>
#include <QtGlobal>

namespace
{
const auto windowsStyle = QStringLiteral(":/platform/windows/onboardingdlg.qss");
const auto macosStyle = QStringLiteral(":/platform/macos/onboardingdlg.qss");

QString onboardingStylePath()
{
#ifdef Q_OS_MACOS
    return macosStyle;
#else
    return windowsStyle;
#endif
}
}

OnboardingDlg::OnboardingDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OnboardingDlg)
{
    static bool resourcesLoaded = []() {
        Q_INIT_RESOURCE(onboarding_res);
        return true;
    }();

    ui->setupUi(this);
    setWindowModality(Qt::ApplicationModal);
    ui->lblHeaderTitle->setText(windowTitle());
    ui->btnIconTitle->setIcon(qApp->windowIcon());
    ui->btnIconTitle->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->lblHeaderTitle->setAttribute(Qt::WA_TransparentForMouseEvents);

#ifdef Q_OS_WINDOWS
    DlgUtils::setTransparent(this);
    DlgUtils::applyDropShadowDialog(ui->frameRoot);
    ui->frameTitle->installEventFilter(this);
#else
    ui->frameTitle->setVisible(false);
#endif

    ui->stackedWidget->addWidget(new OnboardingPage1(ui->stackedWidget));
    ui->stackedWidget->addWidget(new OnboardingPage2(ui->stackedWidget));
    ui->stackedWidget->addWidget(new OnboardingPage3(ui->stackedWidget));
    ui->stackedWidget->addWidget(new OnboardingPage4(ui->stackedWidget));
    ui->stackedWidget->addWidget(new OnboardingPage5(ui->stackedWidget));
    ui->onboardingIndicator->setCount(ui->stackedWidget->count());

    connect(ui->btnSkip, &QPushButton::clicked, this, &OnboardingDlg::skip);
    connect(ui->btnBack, &QPushButton::clicked, this, &OnboardingDlg::showPreviousPage);
    connect(ui->btnNext, &QPushButton::clicked, this, &OnboardingDlg::showNextPage);
    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [this](int page) {
        updateNavigation();
        Q_EMIT pageShown(page);
    });
    connect(APP::Theme::instance(), &APP::Theme::themeChanged, this, &OnboardingDlg::updateTheme);

    updateTheme(APP::Theme::instance()->isDarkTheme());
    updateNavigation();
}

OnboardingDlg::~OnboardingDlg()
{
    delete ui;
}

int OnboardingDlg::currentPage() const
{
    return ui->stackedWidget->currentIndex();
}

void OnboardingDlg::setCurrentPage(int page)
{
    const int normalizedPage = qBound(0, page, ui->stackedWidget->count() - 1);
    ui->stackedWidget->setCurrentIndex(normalizedPage);
    updateNavigation();
}

void OnboardingDlg::closeEvent(QCloseEvent *event)
{
    if (!_completed && !_dismissed && currentPage() < ui->stackedWidget->count() - 1) {
        _dismissed = true;
        Q_EMIT dismissed();
    }

    QDialog::closeEvent(event);
}

bool OnboardingDlg::eventFilter(QObject *watched, QEvent *event)
{
#ifdef Q_OS_WINDOWS
    if (watched == ui->frameTitle) {
        switch (event->type()) {
        case QEvent::MouseButtonPress: {
            const auto mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() != Qt::LeftButton) {
                break;
            }
            _dragStartGlobalPosition = mouseEvent->globalPosition().toPoint();
            _dragStartWindowPosition = frameGeometry().topLeft();
            _dragging = true;
            return true;
        }
        case QEvent::MouseMove: {
            const auto mouseEvent = static_cast<QMouseEvent *>(event);
            if (!_dragging || !(mouseEvent->buttons() & Qt::LeftButton)) {
                break;
            }
            const QPoint globalPosition = mouseEvent->globalPosition().toPoint();
            move(_dragStartWindowPosition + globalPosition - _dragStartGlobalPosition);
            return true;
        }
        case QEvent::MouseButtonRelease:
            _dragging = false;
            return true;
        default:
            break;
        }
    }
#endif

    return QDialog::eventFilter(watched, event);
}

void OnboardingDlg::showPreviousPage()
{
    setCurrentPage(currentPage() - 1);
}

void OnboardingDlg::showNextPage()
{
    if (currentPage() == ui->stackedWidget->count() - 1) {
        _completed = true;
        Q_EMIT completed();
        accept();
        return;
    }

    setCurrentPage(currentPage() + 1);
}

void OnboardingDlg::skip()
{
    _dismissed = true;
    Q_EMIT dismissed();
    reject();
}

void OnboardingDlg::updateTheme(bool isDark)
{
    setStyleSheet(DlgUtils::loadFileToString(onboardingStylePath()));
    DlgUtils::setTheme(this, isDark);
}

void OnboardingDlg::updateNavigation()
{
    const bool firstPage = currentPage() == 0;
    const bool lastPage = currentPage() == ui->stackedWidget->count() - 1;

    ui->btnBack->setEnabled(!firstPage);
    ui->btnNext->setText(lastPage ? tr("Done") : tr("Next >"));
    ui->onboardingIndicator->setCurrentIndex(currentPage());
}
