#pragma once

#include "enums.h"

#include <QWidget>
#include <QProperty>

namespace Ui {class FinishedPage;}

class FinishedPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool darkTheme READ isDarkTheme WRITE setDarkTheme BINDABLE bindableDarkTheme)

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

    APP::Wizard::SyncMode syncMode() const;
    QString syncTargetDir() const;

    bool isDarkTheme() const { return darkTheme_.value(); }
    void setDarkTheme(bool v) { darkTheme_.setValue(v); }

    QBindable<bool> bindableDarkTheme() {return &darkTheme_;}

Q_SIGNALS:
    void backClicked();
    void doneClicked(APP::Wizard::SyncMode syncMode, const QString& targetDir);

private:
    void advancedStateChanged(bool checked);
    void onBrowseClicked();

    Ui::FinishedPage *ui = nullptr;
    QString defaultTargetDir_;
    QPropertyNotifier themeNotifier;
    QProperty<bool> darkTheme_ {false};
};
