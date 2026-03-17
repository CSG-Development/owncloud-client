#pragma once

#include "confirmdlgcontroller.h"
#include "commondialogwidgets.h"

#include <QDialog>
#include <QProperty>
#include <QMetaProperty>

class BaseConfirmDlg : public QDialog
{
    Q_OBJECT

public:
    explicit BaseConfirmDlg(QWidget *parent = nullptr);

    bool eventFilter(QObject *watched, QEvent *event) override;

    BaseConfirmDlg& setHeaderText(const QString& headerText);
    BaseConfirmDlg& setMessageText(const QString& messageText);
    BaseConfirmDlg& setAcceptButtonText(const QString& acceptText);
    BaseConfirmDlg& setRejectButtonText(const QString& rejectText);
    BaseConfirmDlg& setWarningIconVisible(bool visible);
    BaseConfirmDlg& setWide(bool wide);
    BaseConfirmDlg& setDefaultButton(QDialog::DialogCode code);
    BaseConfirmDlg& setSingleButton(bool val);

protected:
    void setupCommonLogic(const CommonDialogWidgets& widgets);

protected:
    virtual void applyTheme(bool isDark);
    virtual void syncUI();

    CommonDialogWidgets _widgets;
    ConfirmDlgController* _ctrl = nullptr;
    std::list<QPropertyNotifier> _notifiers;

};

