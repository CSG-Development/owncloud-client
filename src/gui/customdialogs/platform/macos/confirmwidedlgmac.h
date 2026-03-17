#pragma once

#include "platform/common/baseconfirmdlg.h"

#include <QDialog>

namespace Ui {class ConfirmWideDlgMac;}

class ConfirmDlgController;

class ConfirmWideDlgMac : public BaseConfirmDlg
{
    Q_OBJECT

public:
    explicit ConfirmWideDlgMac(QWidget *parent = nullptr);
    ~ConfirmWideDlgMac();

protected:
    void syncUI() override;
    void applyTheme(bool isDark) override;

private:
    Ui::ConfirmWideDlgMac *ui = nullptr;
};
