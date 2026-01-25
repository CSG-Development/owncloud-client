#pragma once

#include <QProperty>
#include <QWidget>

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

class QGraphicsDropShadowEffect;

class CodeDialogController : public QObject
{
    Q_OBJECT

public:
    QProperty<bool> errorState {false};
    QProperty<bool> codeInputEnabled {false};
    QProperty<bool> btnAllowAccessVisible {false};
    QProperty<bool> btnAllowAccessEnabled {false};
    QProperty<bool> btnResendCodeVisible {false};
    QProperty<bool> btnResendCodeEnabled {false};
    QProperty<bool> spinnerVisible {false};
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

public:
    explicit CodeDialog(QWidget *parent = nullptr);
    ~CodeDialog();

    void updateTheme();

    void reset();
    void setDialogState(CodeDialogState state);
    void setError(CodeDialogState state, const QString& errorStr, const QString& errorTooltip);

    QString getCode() const;
    void clearCode();

signals:
    void codeAction(CodeAction act, const QString& code = QString());

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    static QString CodeDialogStateToStr(CodeDialogState state);

private:
    Ui::CodeDialog *ui = nullptr;
    bool errorState_ = false;
    CodeDialogState state_ = CodeDialogState::AllowAccess;
    QGraphicsDropShadowEffect* shadowEffect = nullptr;
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
