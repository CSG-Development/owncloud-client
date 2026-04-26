#include "toastwidget.h"
#include "ui_toastwidget.h"

#include "stylehelper.h"
#include "theme.h"

#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QShowEvent>

namespace {
const auto widgetStyle = QStringLiteral(":/res/toast/toast.qss");

const QPair<QString, QString> closeIcon = {
    QStringLiteral(":/res/toast/close_light.svg"),
    QStringLiteral(":/res/toast/close_dark.svg")
};

constexpr int horizontalScreenMargin = 28;
constexpr int bottomMargin = 28;
constexpr int minToastWidth = 320;
constexpr int maxToastWidth = 760;
constexpr int minAvailableWidth = 220;
constexpr int minLabelWidth = 140;
constexpr int iconSize = 36;
}

ToastWidget::ToastWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ToastWidget)
    , shadowEffect_(new QGraphicsDropShadowEffect(this))
{
    ui->setupUi(this);

    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFocusPolicy(Qt::NoFocus);

    connect(ui->btnClose, &QToolButton::clicked, this, &ToastWidget::hideMessage);

    ui->btnClose->setIconSize(QSize(iconSize, iconSize));

    shadowEffect_->setXOffset(0);
    shadowEffect_->setYOffset(16);
    shadowEffect_->setBlurRadius(38);
    ui->cardFrame->setGraphicsEffect(shadowEffect_);

    connect(APP::Theme::instance(), &APP::Theme::themeChanged, this, &ToastWidget::updateStyles);

    bindParent();
    updateStyles(APP::Theme::instance()->isDarkTheme());
    hide();
}

ToastWidget::~ToastWidget()
{
    unbindParent();
    delete ui;
}

void ToastWidget::showMessage(const QString &message)
{
    setMessage(message);
    updateGeometryForParent();
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
    updateGeometryForParent();
}

bool ToastWidget::event(QEvent *event)
{
    if (event->type() == QEvent::ParentAboutToChange) {
        unbindParent();
    } else if (event->type() == QEvent::ParentChange) {
        bindParent();
        updateGeometryForParent();
    }

    return QWidget::event(event);
}

bool ToastWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parentWidget()) {
        switch (event->type()) {
        case QEvent::Resize:
        case QEvent::Move:
        case QEvent::Show:
        case QEvent::WindowStateChange:
        case QEvent::LayoutRequest:
            updateGeometryForParent();
            raise();
            break;
        case QEvent::ChildAdded:
            raise();
            break;
        default:
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void ToastWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    updateGeometryForParent();
    raise();
}

void ToastWidget::bindParent()
{
    if (auto *parentWidget_ = parentWidget()) {
        parentWidget_->installEventFilter(this);
    }
}

void ToastWidget::unbindParent()
{
    if (auto *parentWidget_ = parentWidget()) {
        parentWidget_->removeEventFilter(this);
    }
}

void ToastWidget::updateStyles(bool isDark)
{
    setStyleSheet(APP::StyleHelper::loadFileToString(widgetStyle));
    APP::StyleHelper::setTheme(this, isDark);

    ui->btnClose->setIcon(isDark ? QIcon(closeIcon.second) : QIcon(closeIcon.first));

    shadowEffect_->setColor(isDark ? QColor(0, 0, 0, 90) : QColor(0, 0, 0, 46));

    updateGeometryForParent();
}

void ToastWidget::updatePosition()
{
    auto *parentWidget_ = parentWidget();
    if (!parentWidget_) {
        return;
    }

    const int x = qMax(0, (parentWidget_->width() - width()) / 2);
    const int y = qMax(0, parentWidget_->height() - height() - bottomMargin);
    move(x, y);
}

void ToastWidget::updateGeometryForParent()
{
    auto *parentWidget_ = parentWidget();
    if (!parentWidget_) {
        adjustSize();
        return;
    }

    const int parentWidth = qMax(1, parentWidget_->width());
    const int availableWidth = qMax(minAvailableWidth, parentWidth - (horizontalScreenMargin * 2));
    const int preferredWidth = qMin(maxToastWidth, qMax(minToastWidth, availableWidth));
    const int toastWidth = qMin(preferredWidth, qMax(minAvailableWidth, parentWidth));

    const auto outerMargins = ui->verticalLayout->contentsMargins();
    const auto cardMargins = ui->horizontalLayout->contentsMargins();
    const int fixedWidth = outerMargins.left() + outerMargins.right() +
                           cardMargins.left() + cardMargins.right() +
                           ui->horizontalLayout->spacing() + ui->btnClose->sizeHint().width();
    const int labelWidth = qMax(minLabelWidth, toastWidth - fixedWidth);

    ui->lblMessage->setMaximumWidth(labelWidth);
    setFixedWidth(toastWidth);
    adjustSize();
    updatePosition();
}
