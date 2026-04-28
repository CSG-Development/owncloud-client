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
const auto successIcon = QStringLiteral(":/res/toast/success.svg");

constexpr int bottomMargin = 28;
constexpr int iconSize = 12;
constexpr int iconImageSize = 24;
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

    ui->btnIcon->setIconSize(QSize(iconImageSize, iconImageSize));
    ui->btnIcon->setIcon(QIcon(successIcon));
    ui->btnIcon->setAttribute(Qt::WA_TransparentForMouseEvents);

    shadowEffect_->setXOffset(0);
    shadowEffect_->setYOffset(0);
    shadowEffect_->setBlurRadius(20);
    shadowEffect_->setColor(QColor(0, 0, 0, 100));
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
    adjustSize();

    auto *parentWidget_ = parentWidget();
    if (!parentWidget_) {
        return;
    }

    updatePosition();
}
