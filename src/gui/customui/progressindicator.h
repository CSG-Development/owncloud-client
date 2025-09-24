#pragma once

#include <QWidget>
#include <QTime>
#include <QEasingCurve>

class ProgressIndicator: public QWidget
{
    Q_OBJECT

public:
    explicit ProgressIndicator(QWidget *parent = nullptr);

    Q_INVOKABLE void setDarkTheme();

protected:
    void paintEvent(QPaintEvent *event) override;
    void flipHorizontal(QPainter& painter, const QRect& r);

private slots:
    void updateAnimation();

private:
    QTimer* timer_ = nullptr;
    double angle_ = 0;
    int animationTime_ = 0;
    QTime startTime_;
    QEasingCurve easingCurve_;
    bool isDark = false;
};
