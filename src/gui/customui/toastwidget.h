#pragma once

#include "floatingbottombanner.h"

class QGraphicsDropShadowEffect;

namespace Ui {
class ToastWidget;
}

class ToastWidget : public FloatingBottomBanner
{
    Q_OBJECT

public:
    explicit ToastWidget(QWidget *parent = nullptr);
    ~ToastWidget() override;

    void showMessage(const QString &message);
    void hideMessage();

    QString message() const;
    void setMessage(const QString &message);

signals:
    void dismissed();

private:
    void updateStyles(bool isDark);

    Ui::ToastWidget *ui = nullptr;
    QGraphicsDropShadowEffect *shadowEffect_ = nullptr;
};
