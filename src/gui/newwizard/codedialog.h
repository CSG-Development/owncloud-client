#pragma once

#include <QWidget>

namespace Ui { class CodeDialog; }

class CodeDialog : public QWidget
{
    Q_OBJECT

public:
    explicit CodeDialog(QWidget *parent = nullptr);
    ~CodeDialog();

    void updateTheme();
    void showError(const QString& txt);

    QString getCode() const;
    void clearCode();

signals:
    void skipClicked();
    void allowAccessClicked();
    void resendCodeClicked();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::CodeDialog *ui = nullptr;
    bool errorState_ = false;
};
