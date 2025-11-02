#pragma once

#include <QWidget>

namespace Ui {class EmailPage;}

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

Q_SIGNALS:
    void loginClicked(const QString& user);
    void cancelClicked();
    void settingsClicked();

protected:
    void moveEvent(QMoveEvent *event) override;

private:
    void onTextEdited(const QString& txt);

    void validateFormData();

    // EmailValidate and UrlValidate return true if input string is empty to avoid startup errors
    bool simpleEmailValidate(const QString& email);

    bool isAllFieldNotEmpty();

    Ui::EmailPage* ui = nullptr;
};
