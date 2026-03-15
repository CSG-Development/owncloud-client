#pragma once

#include "platform/common/baseinputdlg.h"
#include <QDialog>

namespace Ui {class InputDlg;}

class InputDlg : public BaseInputDlg
{
    Q_OBJECT

public:
    explicit InputDlg(QWidget *parent = nullptr);
    ~InputDlg();

protected:
    void syncUI() override;
    void applyTheme(bool isDark) override;

private:
    Ui::InputDlg *ui = nullptr;

};
