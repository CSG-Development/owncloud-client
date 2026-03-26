#pragma once

#include "enums.h"

#include <QWidget>
#include <QProperty>

namespace Ui {class FinishedPage;}

class QFrame;
class QRadioButton;
class FocusFrame;

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

    APP::Wizard::SyncMode syncMode() const;
    QString syncTargetDir() const;

    bool isDarkTheme() const { return darkTheme_.value(); }
    void setDarkTheme(bool v) { darkTheme_.setValue(v); }

    QBindable<bool> bindableDarkTheme() {return &darkTheme_;}

    bool eventFilter(QObject *watched, QEvent *event) override;

Q_SIGNALS:
    void backClicked();
    void doneClicked(APP::Wizard::SyncMode syncMode, const QString& targetDir);

private:
    void advancedStateChanged(bool checked);
    void onBrowseClicked();

    void handleFrameEvent(QEvent *event, QRadioButton* button);
    bool handleFrameMouse(QEvent *event, QRadioButton* button);
    void handleFocusEvent(QEvent *event, FocusFrame* frame);

    Ui::FinishedPage *ui = nullptr;
    QString defaultTargetDir_;
    QPropertyNotifier themeNotifier;
    QProperty<bool> darkTheme_ {false};
};
