#pragma once

#include "inputdlgcontroller.h"
#include "commondialogwidgets.h"
#include <QDialog>

class BaseInputDlg : public QDialog
{
    Q_OBJECT

public:
    explicit BaseInputDlg(QWidget *parent = nullptr);
    void setRealParent(QWidget *parent) {_realParent = parent;}

    BaseInputDlg& setHeaderText(const QString& headerText);
    BaseInputDlg& setPromptText(const QString& promptText);
    BaseInputDlg& setAcceptButtonText(const QString& acceptText);
    BaseInputDlg& setRejectButtonText(const QString& rejectText);
    BaseInputDlg& setDefaultButton(QDialog::DialogCode code);

    QString inputText() const;

    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void inputTextChanged(const QString& text);

protected:
    void setupCommonLogic(const CommonDialogWidgets& widgets);
    void showEvent(QShowEvent *event) override;

protected:
    virtual void applyTheme(bool isDark);
    virtual void syncUI();

    CommonDialogWidgets _widgets;
    InputDlgController* _ctrl = nullptr;
    std::list<QPropertyNotifier> _notifiers;
    bool _isFirstShow = true;
    QWidget* _realParent = nullptr;
};
