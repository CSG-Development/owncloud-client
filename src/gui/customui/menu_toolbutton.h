#pragma once

#include <QToolButton>
#include <QColor>

namespace APP {

class MenuToolButton: public QToolButton
{
    Q_OBJECT

public:
    explicit MenuToolButton(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event);
    void enterEvent(QEnterEvent *event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    QColor buttonFrameFocused() const;

    QColor buttonFrameNormal() const;
    QColor buttonFramePressed() const;
    QColor buttonFrameHovered() const;
    QColor buttonFrameDisabled() const;

    QColor buttonBackgroundNormal() const;
    QColor buttonBackgroundPressed() const;
    QColor buttonBackgroundHovered() const;
    QColor buttonBackgroundDisabled() const;

    QColor buttonBackgroundCheckedNormal() const;
    QColor buttonBackgroundCheckedPressed() const;
    QColor buttonBackgroundCheckedHovered() const;

    bool isHovered = false;
    bool isPressed = false;
    bool isDark = false;
};

} // namespace APP
