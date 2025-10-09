#pragma once

#include <QWidget>

namespace Ui {class CredentialsPage;}

class CredentialsPage : public QWidget
{
    Q_OBJECT

public:
    explicit CredentialsPage(QWidget *parent = nullptr);
    ~CredentialsPage();

    bool eventFilter(QObject *watched, QEvent *event) override;

    void updateTheme();

    QString url() const;
    QString email() const;
    QString password() const;

    void showErrorMessage(const QString& msg);

Q_SIGNALS:
    void loginClicked(const QString& url, const QString& user, const QString& password);
    void cancelClicked();
    void settingsClicked();
    void resetPasswordClicked();

private:
    void onTextChanged(const QString& txt);
    void onTextEdited(const QString& txt);
    void onEditingFinished();

    void validateFormData();

    // EmailValidate and UrlValidate return true if input string is empty to avoid startup errors
    bool simpleEmailValidate(const QString& email);
    bool simpleUrlValidate(const QString& url);

    bool isAllFieldValid();

    Ui::CredentialsPage* ui = nullptr;
};
