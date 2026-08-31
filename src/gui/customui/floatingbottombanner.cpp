#include "floatingbottombanner.h"

#include <QEvent>
#include <QShowEvent>

namespace {
constexpr int bottomMargin = 28;
}

FloatingBottomBanner::FloatingBottomBanner(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFocusPolicy(Qt::NoFocus);

    bindParent();
    hide();
}

FloatingBottomBanner::~FloatingBottomBanner()
{
    unbindParent();
}

bool FloatingBottomBanner::event(QEvent *event)
{
    if (event->type() == QEvent::ParentAboutToChange) {
        unbindParent();
    } else if (event->type() == QEvent::ParentChange) {
        bindParent();
        refreshGeometryForParent();
    }

    return QWidget::event(event);
}

bool FloatingBottomBanner::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parentWidget()) {
        switch (event->type()) {
        case QEvent::Resize:
        case QEvent::Move:
        case QEvent::Show:
        case QEvent::WindowStateChange:
        case QEvent::LayoutRequest:
            refreshGeometryForParent();
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

void FloatingBottomBanner::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    refreshGeometryForParent();
    raise();
}

void FloatingBottomBanner::bindParent()
{
    if (auto *parentWidget_ = parentWidget()) {
        parentWidget_->installEventFilter(this);
    }
}

void FloatingBottomBanner::unbindParent()
{
    if (auto *parentWidget_ = parentWidget()) {
        parentWidget_->removeEventFilter(this);
    }
}

void FloatingBottomBanner::refreshGeometryForParent()
{
    adjustSize();

    auto *parentWidget_ = parentWidget();
    if (!parentWidget_) {
        return;
    }

    const int x = qMax(0, (parentWidget_->width() - width()) / 2);
    const int y = qMax(0, parentWidget_->height() - height() - bottomMargin);
    move(x, y);
}
