#pragma once

#include <QWidget>
#include <QRegularExpression>
#include <QProperty>

namespace Ui {class EmailPage;}

class FocusProxyStyle;

class EmailPageController : public QObject
{
    Q_OBJECT
public:
    explicit EmailPageController(QObject* parent = nullptr);

    QProperty<QString> email;
    QProperty<QString> buttonTooltip;
    QProperty<bool> isFocused{false};
    QProperty<bool> canLogin {false};
    QProperty<bool> errorState {false};

private:
    bool isEmailValid(const QString& email_text) const {
        if (email_text.trimmed().isEmpty())
            return false;

        const static QRegularExpression rx(QStringLiteral("^[0-9a-zA-Z]+([0-9a-zA-Z]*[-._+])*[0-9a-zA-Z]+@[0-9a-zA-Z]+([-.][0-9a-zA-Z]+)*([0-9a-zA-Z]*[.])[a-zA-Z]{2,6}$"),
                                           QRegularExpression::CaseInsensitiveOption);
        return rx.match(email_text).hasMatch();
    }
};

class EmailPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged FINAL);

public:
    explicit EmailPage(QWidget *parent = nullptr);
    ~EmailPage();

    void updateTheme();

    void setEmail(const QString& email);
    QString email() const;

Q_SIGNALS:
    void loginClicked(const QString& user);
    void cancelClicked();
    void settingsClicked();

    void emailChanged();

private:
    Ui::EmailPage* ui = nullptr;
    EmailPageController* controller_ = nullptr;
    std::list<QPropertyNotifier> notifiers_;
};
