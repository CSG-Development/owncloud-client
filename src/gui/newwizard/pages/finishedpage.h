#pragma once

#include "enums.h"
#include <QWidget>

namespace Ui {class FinishedPage;}

class FinishedPage : public QWidget
{
    Q_OBJECT

public:
    explicit FinishedPage(QWidget *parent = nullptr);

    ~FinishedPage();

    void setupPageDefaults(const QString &defaultSyncTargetDir,
        const QString &userChosenSyncTargetDir,
        bool vfsIsAvailable,
        bool enableVfsByDefault,
        bool vfsModeIsExperimental);

    void updateTheme();
    void showErrorMessage(const QString& msg);

    CUR::Wizard::SyncMode syncMode() const;
    QString syncTargetDir() const;

Q_SIGNALS:
    void backClicked();
    void doneClicked(CUR::Wizard::SyncMode syncMode, const QString& targetDir);

private:
    void advancedStateChanged(bool checked);
    void onBrowseClicked();

    Ui::FinishedPage *ui = nullptr;
    QString defaultTargetDir_;
};
