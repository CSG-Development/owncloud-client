#pragma once

#include <QObject>
#include <QTimer>
#include <QPoint>
#include <QRect>

class CustomToolTip;

class ToolTipManager : public QObject
{
    Q_OBJECT

public:
    explicit ToolTipManager(QObject* parent = nullptr);

    static ToolTipManager* instance();

    // like QToolTip::showText()
    void showText(const QPoint &pos, const QString &text, QWidget *w = nullptr, const QRect &rect = {});

    bool isVisible() const;
    void hideTip();

public slots:
    void onThemeChanged(bool isDark);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void showTip();

    CustomToolTip* tipWidget = nullptr;
    QTimer delayTimer;
    QTimer hideTimer;
    QString pendingText;
    QPoint pendingPos;
    QRect targetRect;
    QWidget* activeWidget = nullptr;
};
