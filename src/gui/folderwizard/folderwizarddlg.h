#pragma once

#include <QDialog>

#include "accountfwd.h"
#include "gui/folderwizard/folderwizard.h"

namespace Ui
{
class FolderWizardDlg;
}

namespace APP {

class FolderWizardDlg : public QDialog
{
    Q_OBJECT

public:
    explicit FolderWizardDlg(const AccountStatePtr &account, QWidget *parent = nullptr);
    ~FolderWizardDlg() override;

    FolderWizard::Result result();

    void present();

private:
    void updateTheme(bool isDark);
    void updateNavigation();

    ::Ui::FolderWizardDlg *ui = nullptr;
    FolderWizard *_wizard = nullptr;
};

} // namespace APP
