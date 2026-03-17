#pragma once

#include "platform/common/baseconfirmdlg.h"
#include <QDialog>

namespace Ui {class ConfirmDlg;}

class ConfirmDlg : public BaseConfirmDlg
{
    Q_OBJECT

public:
    explicit ConfirmDlg(QWidget *parent = nullptr);
    ~ConfirmDlg();

protected:
    void syncUI() override;
    void applyTheme(bool isDark) override;

private:
    Ui::ConfirmDlg *ui = nullptr;
};
