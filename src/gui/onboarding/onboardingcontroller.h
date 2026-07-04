#pragma once

#include "onboardingstate.h"

#include <QObject>
#include <QPointer>

class QWidget;
class OnboardingDlg;

namespace APP {

class OnboardingController : public QObject
{
    Q_OBJECT

public:
    explicit OnboardingController(QObject *parent = nullptr);

    bool maybeShow(QWidget *parent = nullptr);
    bool showForDebug(QWidget *parent = nullptr);
    bool isShowing() const;
    void dismissPermanently();

private:
    bool show(QWidget *parent, bool persistState);

    OnboardingState _state;
    QPointer<OnboardingDlg> _dialog;
};

} // namespace APP
