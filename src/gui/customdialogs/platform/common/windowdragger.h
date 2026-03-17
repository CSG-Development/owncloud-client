#pragma once

#include <QObject>
#include <QPointF>
#include <QWidget>
#include <QMouseEvent>

class WindowDragger : public QObject
{
    Q_OBJECT

public:
    explicit WindowDragger(QWidget *handle, QWidget *target = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QWidget *_target = nullptr;
    bool _moving = false;
    QPointF _clickedPos;
    QPoint _windowPos;
};