#include "deviceunreachablebanner.h"
#include "ui_deviceunreachablebanner.h"

#include "stylehelper.h"
#include "theme.h"

namespace {
const auto widgetStyle = QStringLiteral(":/res/toast/device_unreachable_banner.qss");
const auto warningIcon = QStringLiteral(":/res/toast/warning.svg");

constexpr int iconImageSize = 24;
constexpr int maxCardWidth = 580;
constexpr int minCardWidth = 320;
constexpr int sideInset = 8;
}

DeviceUnreachableBanner::DeviceUnreachableBanner(QWidget *parent)
    : FloatingBottomBanner(parent)
    , ui(new Ui::DeviceUnreachableBanner)
{
    ui->setupUi(this);

    ui->lblTitle->setText(tr("Connection lost with Home Cloud"));
    ui->lblSubtitle->setText(tr("Please check the device is powered on and connected to the network."));
    ui->btnRetry->setText(tr("Retry"));

    ui->btnIcon->setIconSize(QSize(iconImageSize, iconImageSize));
    ui->btnIcon->setIcon(QIcon(warningIcon));
    ui->btnIcon->setAttribute(Qt::WA_TransparentForMouseEvents);

    connect(ui->btnRetry, &QPushButton::clicked, this, &DeviceUnreachableBanner::retryClicked);

    connect(APP::Theme::instance(), &APP::Theme::themeChanged, this, &DeviceUnreachableBanner::updateStyles);

    updateStyles(APP::Theme::instance()->isDarkTheme());
    hide();
}

DeviceUnreachableBanner::~DeviceUnreachableBanner()
{
    delete ui;
}

void DeviceUnreachableBanner::updateStyles(bool isDark)
{
    setStyleSheet(APP::StyleHelper::loadFileToString(widgetStyle));
    APP::StyleHelper::setTheme(this, isDark);

    refreshGeometryForParent();
}

void DeviceUnreachableBanner::refreshGeometryForParent()
{
    if (auto *parentWidget_ = parentWidget()) {
        const int available = parentWidget_->width() - 2 * sideInset;
        const int width = qBound(minCardWidth, available, maxCardWidth);
        ui->cardFrame->setMinimumWidth(width);
        ui->cardFrame->setMaximumWidth(width);
    }

    FloatingBottomBanner::refreshGeometryForParent();
}
