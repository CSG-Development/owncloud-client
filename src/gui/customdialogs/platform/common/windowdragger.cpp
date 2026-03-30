#include "windowdragger.h"

WindowDragger::WindowDragger(QWidget *handle, QWidget *target)
    : QObject(handle)
    , _target(target)
{
    if (!_target) {
        _target = handle->window();
    }
    handle->installEventFilter(this);
}

bool WindowDragger::eventFilter(QObject *watched, QEvent *event)
{
    auto* mouseEvent = static_cast<QMouseEvent*>(event);

    switch (event->type()) {
    case QEvent::MouseButtonPress:
        if (mouseEvent->button() == Qt::LeftButton) {
            _clickedPos = mouseEvent->globalPosition();
            _windowPos = _target->pos();
            _moving = true;
            return true;
        }
        break;

    case QEvent::MouseMove:
        if (_moving) {
            QPointF delta = mouseEvent->globalPosition() - _clickedPos;
            _target->move(_windowPos + delta.toPoint());
            return true;
        }
        break;

    case QEvent::MouseButtonRelease:
        if (_moving && mouseEvent->button() == Qt::LeftButton) {
            _moving = false;
            return true;
        }
        break;

    default:
        break;
    }

    return QObject::eventFilter(watched, event);
}
