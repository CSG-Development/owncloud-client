#pragma once

#include <QWidget>

namespace APP {

class FolderWizard;

class FolderWizardPage : public QWidget
{
    Q_OBJECT
public:
    explicit FolderWizardPage(FolderWizard *wizard)
        : QWidget(nullptr)
        , _wizard(wizard)
    {
    }

    virtual void initializePage() { }
    virtual void cleanupPage() { }
    virtual bool validatePage() { return true; }
    virtual bool isComplete() const { return true; }

Q_SIGNALS:
    void completeChanged();

protected:
    FolderWizard *folderWizard() const { return _wizard; }

private:
    FolderWizard *_wizard;
};

} // namespace APP
