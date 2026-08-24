#include "toastwidget.h"
#include "ui_toastwidget.h"

#include "stylehelper.h"
#include "theme.h"

#include <QGraphicsDropShadowEffect>

namespace {
const auto widgetStyle = QStringLiteral(":/res/toast/toast.qss");

const QPair<QString, QString> closeIcon = {
    QStringLiteral(":/res/toast/close_light.svg"),
    QStringLiteral(":/res/toast/close_dark.svg")
};
const auto successIcon = QStringLiteral(":/res/toast/success.svg");

constexpr int iconSize = 12;
constexpr int iconImageSize = 24;
}

ToastWidget::ToastWidget(QWidget *parent)
    : FloatingBottomBanner(parent)
    , ui(new Ui::ToastWidget)
    , shadowEffect_(new QGraphicsDropShadowEffect(this))
{
    ui->setupUi(this);

    connect(ui->btnClose, &QToolButton::clicked, this, &ToastWidget::hideMessage);

    ui->btnClose->setIconSize(QSize(iconSize, iconSize));

    ui->btnIcon->setIconSize(QSize(iconImageSize, iconImageSize));
    ui->btnIcon->setIcon(QIcon(successIcon));
    ui->btnIcon->setAttribute(Qt::WA_TransparentForMouseEvents);

    shadowEffect_->setXOffset(0);
    shadowEffect_->setYOffset(0);
    shadowEffect_->setBlurRadius(20);
    shadowEffect_->setColor(QColor(0, 0, 0, 100));
    ui->cardFrame->setGraphicsEffect(shadowEffect_);

    connect(APP::Theme::instance(), &APP::Theme::themeChanged, this, &ToastWidget::updateStyles);

    updateStyles(APP::Theme::instance()->isDarkTheme());
    hide();
}

ToastWidget::~ToastWidget()
{
    delete ui;
}

void ToastWidget::showMessage(const QString &message)
{
    setMessage(message);
    refreshGeometryForParent();
    show();
    raise();
}

void ToastWidget::hideMessage()
{
    if (!isVisible()) {
        return;
    }

    hide();
    emit dismissed();
}

QString ToastWidget::message() const
{
    return ui->lblMessage->text();
}

void ToastWidget::setMessage(const QString &message)
{
    ui->lblMessage->setText(message);
    refreshGeometryForParent();
}

void ToastWidget::updateStyles(bool isDark)
{
    setStyleSheet(APP::StyleHelper::loadFileToString(widgetStyle));
    APP::StyleHelper::setTheme(this, isDark);

    ui->btnClose->setIcon(isDark ? QIcon(closeIcon.second) : QIcon(closeIcon.first));
    shadowEffect_->setColor(isDark ? QColor(0, 0, 0, 90) : QColor(0, 0, 0, 46));

    refreshGeometryForParent();
}
