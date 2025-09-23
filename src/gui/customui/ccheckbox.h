#pragma once

#include <QCheckBox>
#include <QPainterPath>

class CCheckBox: public QCheckBox
{
    Q_OBJECT

public:
    explicit CCheckBox(QWidget* parent = nullptr);
    explicit CCheckBox(const QString& text, QWidget* parent = nullptr);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    Q_INVOKABLE void setDarkTheme();

protected:
    void paintEvent(QPaintEvent *event) override;
    bool hitButton(const QPoint &pos) const override;

    QPainterPath focusFrame(const QRectF& r);

private:
    QSize iconSize_ {20, 20};
    int frameWidth_ = 2;
    int frameRadius_ = 7;
    QColor frameColor_;
    QColor textColor_;
    bool isDark = false;
};
