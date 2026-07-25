#pragma once

#include <QProperty>
#include <QWidget>
#include <utility>

namespace Ui { class CodeDialog; }

enum class CodeDialogState {
    Waiting,
    AllowAccess,
    Resend
};

enum class CodeAction {
    Entered,
    Resend,
    Skip
};

enum class CodeErrorState {
    None,
    CodeInvalid,
    CodeExpired
};

class QGraphicsDropShadowEffect;
class DimWidget;

class CodeDialogController : public QObject
{
    Q_OBJECT

public:
    QProperty<CodeErrorState> errorState {CodeErrorState::None};
    QProperty<bool> codeInputEnabled {false};
    QProperty<bool> btnAllowAccessVisible {false};
    QProperty<bool> btnAllowAccessEnabled {false};
    QProperty<bool> btnResendCodeVisible {false};
    QProperty<bool> btnResendCodeEnabled {false};
    QProperty<bool> spinnerVisible {false};
    QProperty<bool> darkTheme{false};
    QProperty<CodeDialogState> state {CodeDialogState::AllowAccess};
    QProperty<QString> errorString {};
    QProperty<QString> errorTooltip {};
    QProperty<QString> codeString {};

    explicit CodeDialogController(QObject *parent = nullptr);

    bool isCodeValid() const {return codeString.value().length() == 6;}
    void clearError();
};

class CodeDialog : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool darkTheme READ isDarkTheme WRITE setDarkTheme BINDABLE bindableDarkTheme)

public:
    explicit CodeDialog(QWidget *parent = nullptr);
    ~CodeDialog();

    void reset();
    void setDialogState(CodeDialogState state);
    void setError(CodeErrorState errorState);

    QString getCode() const;
    void clearCode();
    void focusCodeInput();

    bool isDarkTheme() const { return controller_->darkTheme.value(); }
    void setDarkTheme(bool v) { controller_->darkTheme.setValue(v); }
    QBindable<bool> bindableDarkTheme() {return &controller_->darkTheme;}

signals:
    void codeAction(CodeAction act, const QString& code = QString());

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject* obj, QEvent* ev) override;

private:
    static QString CodeDialogStateToStr(CodeDialogState state);

private:
    Ui::CodeDialog *ui = nullptr;
    bool errorState_ = false;
    CodeDialogState state_ = CodeDialogState::AllowAccess;
    CodeDialogController* controller_ = nullptr;
    std::list<QPropertyNotifier> notifiers_;
};

inline QString CodeActionToStr(CodeAction act) {
    QMap<CodeAction,QString> map = {
        {CodeAction::Entered,QStringLiteral("CodeAction::Entered")},
        {CodeAction::Resend,QStringLiteral("CodeAction::Resend")},
        {CodeAction::Skip,QStringLiteral("CodeAction::Skip")},
    };
    if (map.contains(act))
        return map[act];
    return {};
}
