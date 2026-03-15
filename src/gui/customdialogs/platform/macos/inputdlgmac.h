#pragma once

#include "platform/common/baseinputdlg.h"
#include <QDialog>

namespace Ui {class InputDlgMac;}

class InputDlgMac : public BaseInputDlg
{
    Q_OBJECT

public:
    explicit InputDlgMac(QWidget *parent = nullptr);
    ~InputDlgMac();

protected:
    void syncUI() override;
    void applyTheme(bool isDark) override;

private:
    Ui::InputDlgMac *ui = nullptr;
    QAction* clearAction = nullptr;
};
