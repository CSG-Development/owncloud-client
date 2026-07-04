#include "onboardingcontroller.h"

#include "onboardingdlg.h"

#include <QWidget>

namespace APP {

OnboardingController::OnboardingController(QObject *parent)
    : QObject(parent)
{

}

bool OnboardingController::maybeShow(QWidget *parent)
{
    if (!_state.shouldShow()) {
        return false;
    }

    return show(parent, true);
}

bool OnboardingController::showForDebug(QWidget *parent)
{
    return show(parent, false);
}

void OnboardingController::dismissPermanently()
{
    _state.markDismissed();
    if (_dialog) {
        _dialog->reject();
    }
}

bool OnboardingController::show(QWidget *parent, bool persistState)
{
    if (_dialog) {
        _dialog->raise();
        _dialog->activateWindow();
        return true;
    }

    auto *dialog = new OnboardingDlg(parent);
    _dialog = dialog;

    if (persistState) {
        connect(dialog, &OnboardingDlg::pageShown, this, [this](int page) {
            _state.markPageShown(page);
        });
        connect(dialog, &OnboardingDlg::completed, this, [this] {
            _state.markCompleted();
        });
        connect(dialog, &OnboardingDlg::dismissed, this, [this] {
            _state.markDismissed();
        });
    }
    connect(dialog, &QDialog::finished, this, [this, dialog] {
        if (_dialog == dialog) {
            _dialog = nullptr;
        }
        dialog->deleteLater();
    });

    const int startPage = persistState ? _state.nextPage() : 0;
    dialog->setCurrentPage(startPage);
    if (persistState) {
        _state.markPageShown(startPage);
    }

    dialog->show();
    dialog->raise();
    dialog->activateWindow();

    return true;
}

bool OnboardingController::isShowing() const
{
    return !_dialog.isNull();
}

} // namespace APP
