#pragma once

#include <QWidget>

class QGraphicsDropShadowEffect;

namespace Ui {
class ToastWidget;
}

class ToastWidget : public QWidget
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

protected:
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void bindParent();
    void unbindParent();
    void updateStyles(bool isDark);
    void updatePosition();
    void updateGeometryForParent();

    Ui::ToastWidget *ui = nullptr;
    QGraphicsDropShadowEffect *shadowEffect_ = nullptr;
};
