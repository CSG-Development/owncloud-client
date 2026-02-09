#pragma once

#include <QWidget>
#include <QProperty>

namespace Ui { class ErrorDialog; }

enum class ErrorDialogState {
    TooManyAttempts,
    UnableToConnectInit,
    UnableToConnectToken,
    EmailNotRegistered
};

enum class ErrorAction {
    Ok,
    Retry,
    Cancel
};

class ErrorDialogController : public QObject
{
    Q_OBJECT

public:
    QProperty<bool> darkTheme{false};
    QProperty<ErrorDialogState> state {ErrorDialogState::UnableToConnectInit};
    QProperty<bool> okBtnVisible{false};
    QProperty<bool> retryBtnVisible{false};
    QProperty<bool> cancelBtnVisible{false};
    QProperty<QString> headerText;
    QProperty<QString> contentText;

    explicit ErrorDialogController(QObject *parent = nullptr);
};


class ErrorDialog : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool darkTheme READ isDarkTheme WRITE setDarkTheme BINDABLE bindableDarkTheme)

public:
    explicit ErrorDialog(QWidget *parent = nullptr);
    ~ErrorDialog();

    void setDialogState(ErrorDialogState state);

    bool isDarkTheme() const { return controller_->darkTheme.value(); }
    void setDarkTheme(bool v) { controller_->darkTheme.setValue(v); }
    QBindable<bool> bindableDarkTheme() {return &controller_->darkTheme;}

signals:
    void errorAction(ErrorAction act, ErrorDialogState state);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject* obj, QEvent* ev) override;

private:
    Ui::ErrorDialog* ui = nullptr;
    ErrorDialogController* controller_ = nullptr;
    std::list<QPropertyNotifier> notifiers_;
};

inline QString ErrorDialogStateToStr(ErrorDialogState state) {
    QMap<ErrorDialogState, QString> map = {
        {ErrorDialogState::EmailNotRegistered, QStringLiteral("ErrorDialogState::EmailNotRegistered")},
        {ErrorDialogState::TooManyAttempts, QStringLiteral("ErrorDialogState::TooManyAttempts")},
        {ErrorDialogState::UnableToConnectInit, QStringLiteral("ErrorDialogState::UnableToConnectInit")},
        {ErrorDialogState::UnableToConnectToken, QStringLiteral("ErrorDialogState::UnableToConnectToken")},
    };
    if (map.contains(state))
        return map[state];
    return {};
}
