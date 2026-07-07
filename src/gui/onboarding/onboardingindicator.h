#pragma once

#include <QColor>
#include <QWidget>

class OnboardingIndicator : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int count READ count WRITE setCount)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex)

public:
    explicit OnboardingIndicator(QWidget *parent = nullptr);

    int count() const;
    void setCount(int count);

    int currentIndex() const;
    void setCurrentIndex(int index);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor pointColor() const;

    int _count = 0;
    int _currentIndex = 0;
};
