#pragma once

#include <QWidget>

namespace Ui { class CodeDialog; }

enum class CodeDialogState {
    Startup,
    Waiting,
    AllowAccess,
    Resend
};

class CodeDialog : public QWidget
{
    Q_OBJECT

public:
    explicit CodeDialog(QWidget *parent = nullptr);
    ~CodeDialog();

    void updateTheme();

    void setDialogState(CodeDialogState state);

    void showCodeExpiredError();
    void showInvalidCodeError();
    void showServerError();
    void clearError();

    QString getCode() const;
    void clearCode();

signals:
    void skipClicked();
    void allowAccessClicked();
    void resendCodeClicked();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void onAllowAccessClicked();
    void onResendCodeClicked();

    void showResendButton();
    void showAllowButton();

private:
    Ui::CodeDialog *ui = nullptr;
    bool errorState_ = false;
    CodeDialogState state_ = CodeDialogState::Startup;
};
