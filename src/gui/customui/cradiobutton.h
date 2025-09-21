#pragma once

#include <QRadioButton>
#include <QPainterPath>

class CRadioButton: public QRadioButton
{
    Q_OBJECT
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize NOTIFY iconSizeChanged FINAL)
    Q_PROPERTY(int frameWidth READ frameWidth WRITE setFrameWidth NOTIFY frameWidthChanged FINAL)
    Q_PROPERTY(int frameRadius READ frameRadius WRITE setFrameRadius NOTIFY frameRadiusChanged FINAL)
    Q_PROPERTY(QColor frameColor READ frameColor WRITE setFrameColor NOTIFY frameColorChanged FINAL)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged FINAL)

public:
    explicit CRadioButton(QWidget* parent = nullptr);
    explicit CRadioButton(const QString &text, QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    const QSize& iconSize() const {return iconSize_;}
    void setIconSize(const QSize& val) {iconSize_ = val; Q_EMIT iconSizeChanged();}

    int frameWidth() const {return frameWidth_;}
    void setFrameWidth(int val) {frameWidth_ = val; Q_EMIT frameWidthChanged();}

    int frameRadius() const {return frameRadius_;}
    void setFrameRadius(int val) {frameRadius_ = val; Q_EMIT frameRadiusChanged();}

    QColor frameColor() const {return frameColor_;}
    void setFrameColor(const QColor& val) {frameColor_ = val; Q_EMIT frameColorChanged();}

    QColor textColor() const {return textColor_;}
    void setTextColor(const QColor& val) {textColor_ = val; Q_EMIT textColorChanged();}

Q_SIGNALS:
    void iconSizeChanged();
    void frameWidthChanged();
    void frameRadiusChanged();
    void frameColorChanged();
    void textColorChanged();

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
};
