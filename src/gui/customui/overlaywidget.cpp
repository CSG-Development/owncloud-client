#include "overlaywidget.h"

#include <QEvent>
#include <QResizeEvent>

void OverlayWidget::newParent()
{
    if (!parent())
    {
        qWarning() << "[OverlayWidget] no parent";
        return;
    }
    parent()->installEventFilter(this);
    raise();
}

OverlayWidget::OverlayWidget(QWidget* parent)
    : QWidget{parent}
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setWindowFlag(Qt::X11BypassWindowManagerHint);
    installEventFilter(this);
    newParent();
}

bool OverlayWidget::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == parent())
    {
        if (ev->type() == QEvent::Resize)
        {
            const auto s = static_cast<QResizeEvent*>(ev)->size();
            resize(s);
        }
        else if (ev->type() == QEvent::ChildAdded)
        {
            raise();
        }
    }
    else if (obj == this)
    {
        if (ev->type() == QEvent::Show)
        {
            if (parent())
            {
                const auto parent_widget = qobject_cast<QWidget*>(parent());
                if (parent_widget)
                {
                    resize(parent_widget->size());
                }
            }
        }
    }
    return QWidget::eventFilter(obj, ev);
}

bool OverlayWidget::event(QEvent* ev)
{
    if (ev->type() == QEvent::ParentAboutToChange)
    {
        if (parent())
            parent()->removeEventFilter(this);
    }
    else if (ev->type() == QEvent::ParentChange)
    {
        newParent();
    }
    return QWidget::event(ev);
}
