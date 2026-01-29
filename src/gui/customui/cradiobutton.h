#pragma once

#include <QRadioButton>
#include <QPainterPath>
#include <QProperty>

class CRadioButton: public QRadioButton
{
    Q_OBJECT
    Q_PROPERTY(bool darkTheme READ isDarkTheme WRITE setDarkTheme BINDABLE bindableDarkTheme)

public:
    explicit CRadioButton(QWidget* parent = nullptr);
    explicit CRadioButton(const QString &text, QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    bool isDarkTheme() const { return darkTheme_.value(); }
    void setDarkTheme(bool v) { darkTheme_.setValue(v); }

    QBindable<bool> bindableDarkTheme() {return &darkTheme_;}

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
    QPropertyNotifier themeNotifier;
    QProperty<bool> darkTheme_ {false};
};
