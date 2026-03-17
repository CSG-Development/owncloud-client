#pragma once

#include <QFrame>
#include <QPainter>

struct FrameData {
    int radius = 0;
    double width = 0;
    QColor color;
    QColor background;
    int margin = 0;
};

class FocusFrame : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(double frameWidth READ frameWidth WRITE setFrameWidth)
    Q_PROPERTY(int frameRadius READ frameRadius WRITE setFrameRadius)
    Q_PROPERTY(QColor frameColor READ frameColor WRITE setFrameColor)
    Q_PROPERTY(QColor frameBackground READ frameBackground WRITE setFrameBackground)
    Q_PROPERTY(int frameMargin READ frameMargin WRITE setFrameMargin)

public:
    explicit FocusFrame(QWidget* parent = nullptr)
        : QFrame(parent)
    {
    }

    void setFrameData(const FrameData& d) {
        _data = d;
        setContentsMargins(_data.width + _data.margin, _data.width + _data.margin, _data.width + _data.margin, _data.width + _data.margin);
        update();
    }

    double frameWidth() const {return _data.width;}
    void setFrameWidth(double val)
    {
        _data.width = val;
        setContentsMargins(val, val, val, val);
        update();
    }

    int frameRadius() const {return _data.radius;}
    void setFrameRadius(int val) {_data.radius = val; update();}

    int frameMargin() const {return _data.margin;}
    void setFrameMargin(int val) {_data.margin = val; update();}

    QColor frameColor() const {return _data.color;}
    void setFrameColor(const QColor& val) {_data.color = val; update();}

    QColor frameBackground() const {return _data.background;}
    void setFrameBackground(const QColor& val) {_data.background = val; update();}

protected:
    void paintEvent(QPaintEvent *) override
    {
        bool focused = property("focused").toBool();
        if (focused) {
            QPainter p(this);
            p.setRenderHint(QPainter::Antialiasing);

            p.setPen(QPen(frameColor(), frameWidth()));

            p.setBrush(frameBackground());

            QRectF r = this->rect();
            r.adjust(frameWidth()/2, frameWidth()/2, -frameWidth()/2, -frameWidth()/2);

            p.drawRoundedRect(r, frameRadius(), frameRadius());
        }
    }

private:
    FrameData _data;
};