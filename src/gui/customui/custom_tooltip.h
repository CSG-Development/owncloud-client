#pragma once

#include <QWidget>

class QGraphicsDropShadowEffect;
class QLabel;
class QFrame;

class CustomToolTip : public QWidget
{
public:
    explicit CustomToolTip(QWidget* parent = nullptr);

    QPoint offset() const;
    void setText(const QString& text);

    QMargins shadowMargins() const;
    // Used instead show() to ensure adjust widget size
    void doShow();

public slots:
    void onThemeChanged(bool isDark);

private:
    QFrame* container = nullptr;
    QLabel* label = nullptr;
    QGraphicsDropShadowEffect* shadowEffect = nullptr;
    QPoint currentOffset;
    QMargins shadowMargins_ {30, 30, 30, 30};
};
