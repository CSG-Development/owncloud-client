#pragma once

#include <QWidget>
#include <QTimer>
#include <QDateTime>

namespace Ui {class EmailPage;}

class DimWidget;

class EmailPage : public QWidget
{
    Q_OBJECT

public:
    explicit EmailPage(QWidget *parent = nullptr);
    ~EmailPage();

    bool eventFilter(QObject *watched, QEvent *event) override;

    void updateTheme();

    QString email() const;

    void showErrorMessage(const QString& msg);
    void showInvalidCredentialsError();

    void showCodeDialog();

Q_SIGNALS:
    void loginClicked(const QString& user);
    void cancelClicked();
    void settingsClicked();
    void codeExpired();
    void codeEntered(const QString& code);

private:
    void onTextEdited(const QString& txt);
    void onCodeExpireCheckTimer();

    void validateFormData();

    // EmailValidate and UrlValidate return true if input string is empty to avoid startup errors
    bool simpleEmailValidate(const QString& email);

    bool isAllFieldNotEmpty();

    Ui::EmailPage* ui = nullptr;
    DimWidget* dim = nullptr;
    QTimer codeExpireCheckTimer;
    QDateTime codeExpireTime;
};
