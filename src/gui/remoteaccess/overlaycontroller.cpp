#include "overlaycontroller.h"


OverlayController::OverlayController(QWidget *parent)
    : QObject(parent)
    , _codeDlg(new CodeDialog(parent))
    , _errorDlg(new ErrorDialog(parent))
{
    _codeDlg->setVisible(false);
    _errorDlg->setVisible(false);

    connect(_codeDlg, &CodeDialog::codeAction, this, &OverlayController::onCodeAction);
    connect(_errorDlg, &ErrorDialog::errorAction, this, &OverlayController::onErrorAction);

}

bool OverlayController::requestAccessCode(const QUuid &id, bool clear)
{
    id_ = id;
    showCodeDialogInternal(CodeDialogState::AllowAccess, clear);
    return true;
}

void OverlayController::resendAccessCode(const QUuid& id)
{
    id_ = id;
    showCodeDialogInternal(CodeDialogState::Resend, false);
    setCodeError(CodeErrorState::None);
}

void OverlayController::invalidAccessCode(const QUuid &id)
{
    id_ = id;
    showCodeDialogInternal(CodeDialogState::AllowAccess, false);
    setCodeError(CodeErrorState::CodeInvalid);
}

void OverlayController::expiredAccessCode(const QUuid &id)
{
    id_ = id;
    showCodeDialogInternal(CodeDialogState::Resend, false);
    setCodeError(CodeErrorState::CodeExpired);
}

void OverlayController::retryAccessCode(const QUuid &id)
{
    id_ = id;
    showCodeDialogInternal(CodeDialogState::AllowAccess, false);
    _codeDlg->setDialogState(CodeDialogState::Waiting);
    emit codeEntered(_codeDlg->getCode(), id_);
}

void OverlayController::setCodeError(CodeErrorState errorState)
{
    if (_codeDlg->isVisible())
        _codeDlg->setError(errorState);
}

bool OverlayController::reportError(ErrorDialogState state, const QUuid &id)
{
    bool transitionAllowed = _codeDlg->isVisible() || !isBusy();
    if (!transitionAllowed)
        return false;

    id_ = id;
    showErrorDialogInternal(state);
    return true;
}

void OverlayController::hideAll()
{
    _codeDlg->hide();
    _errorDlg->hide();
    id_ = QUuid();
}

bool OverlayController::isBusy() const
{
    return _codeDlg->isVisible() || _errorDlg->isVisible();
}

void OverlayController::onCodeAction(CodeAction action, const QString &code)
{
    switch (action) {
    case CodeAction::Entered:
        _codeDlg->setDialogState(CodeDialogState::Waiting);
        emit codeEntered(code, id_);
        break;
    case CodeAction::Resend:
        _codeDlg->setDialogState(CodeDialogState::Waiting);
        emit resendRequested(id_);
        break;
    case CodeAction::Skip:
        // hideAll();
        emit processSkipped(id_);
        break;
    }
}

void OverlayController::onErrorAction(ErrorAction action, ErrorDialogState state)
{
    switch (action) {
    case ErrorAction::Retry:
        hideAll();
        emit errorRetry(state, id_);
        break;

    case ErrorAction::Ok:
        hideAll();
        emit errorOk(state, id_);
        break;

    case ErrorAction::Cancel:
        hideAll();
        emit errorCancel(state, id_);
        break;
    }
}

void OverlayController::showCodeDialogInternal(CodeDialogState code_state, bool clear_code)
{
    if (_errorDlg->isVisible())
        _errorDlg->hide();

    _codeDlg->setDialogState(code_state);
    if (clear_code)
        _codeDlg->clearCode();
    _codeDlg->raise();
    _codeDlg->show();
}

void OverlayController::showErrorDialogInternal(ErrorDialogState error_state)
{
    if (_codeDlg->isVisible())
        _codeDlg->hide();

    _errorDlg->setDialogState(error_state);
    _errorDlg->raise();
    _errorDlg->show();
}
