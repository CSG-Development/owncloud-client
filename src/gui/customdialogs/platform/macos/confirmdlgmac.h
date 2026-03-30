#pragma once

#include "platform/common/baseconfirmdlg.h"

#include <QDialog>

namespace Ui {class ConfirmDlgMac;}

class ConfirmDlgMac : public BaseConfirmDlg
{
    Q_OBJECT

public:
    explicit ConfirmDlgMac(QWidget *parent = nullptr);
    ~ConfirmDlgMac();

protected:
    void syncUI() override;
    void applyTheme(bool isDark) override;

private:
    Ui::ConfirmDlgMac *ui = nullptr;
};
