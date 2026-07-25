#pragma once

#include "codedialog.h"
#include "gui/remoteaccess/errordialog.h"

#include <QWidget>
#include <QUuid>

class OverlayController : public QObject
{
    Q_OBJECT

public:
    explicit OverlayController(QWidget *parent = nullptr);

    bool requestAccessCode(const QUuid& id, bool clear);
    void resendAccessCode(const QUuid& id);
    void invalidAccessCode(const QUuid& id);
    void expiredAccessCode(const QUuid& id);
    void retryAccessCode(const QUuid& id);

    void setCodeError(CodeErrorState errorState);
    void focusAccessCodeInput();

    bool reportError(ErrorDialogState state, const QUuid& id);
    void hideAll();

    bool isBusy() const;

    bool ownsCodeDialog(const QUuid& id) const;
    bool ownsOverlay(const QUuid& id) const;

signals:
    void codeEntered(const QString &code, const QUuid& id);
    void processSkipped(const QUuid& id);
    void resendRequested(const QUuid& id);

    void errorRetry(ErrorDialogState state, const QUuid& id);
    void errorCancel(ErrorDialogState state, const QUuid& id);
    void errorOk(ErrorDialogState state, const QUuid& id);

private slots:
    void onCodeAction(CodeAction action, const QString &code);
    void onErrorAction(ErrorAction action, ErrorDialogState state);

private:
    void showCodeDialogInternal(CodeDialogState code_state, bool clear_code);
    void showErrorDialogInternal(ErrorDialogState error_state);

    CodeDialog *_codeDlg = nullptr;
    ErrorDialog *_errorDlg = nullptr;
    QUuid id_;
};