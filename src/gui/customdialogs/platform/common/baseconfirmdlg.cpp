#include "baseconfirmdlg.h"
#include "pushbuttonstyle.h"
#include "dlgutils.h"
#include "windowdragger.h"
#include "prophelper.h"
#include "gui/customui/focusproxy.h"
#include "theme.h"

#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QLayout>
#include <QTimer>

BaseConfirmDlg::BaseConfirmDlg(QWidget *parent)
    : QDialog(parent)
    , _ctrl(new ConfirmDlgController(this))
{
}

bool BaseConfirmDlg::eventFilter(QObject *watched, QEvent *event)
{
    bool need_update = false;
    if (watched == _widgets.btnAccept && _widgets.frameAcceptBtn) {
        if (event->type() == QEvent::FocusIn) {
            _widgets.frameAcceptBtn->setProperty("focused", true);
        }
        else if (event->type() == QEvent::FocusOut) {
            _widgets.frameAcceptBtn->setProperty("focused", false);
        }
        _widgets.frameAcceptBtn->style()->unpolish(_widgets.frameAcceptBtn);
        _widgets.frameAcceptBtn->style()->polish(_widgets.frameAcceptBtn);
        need_update = true;
    }
    else if (watched == _widgets.btnReject && _widgets.frameRejectBtn) {
        if (event->type() == QEvent::FocusIn) {
            _widgets.frameRejectBtn->setProperty("focused", true);
        }
        else if (event->type() == QEvent::FocusOut) {
            _widgets.frameRejectBtn->setProperty("focused", false);
        }
        _widgets.frameRejectBtn->style()->unpolish(_widgets.frameRejectBtn);
        _widgets.frameRejectBtn->style()->polish(_widgets.frameRejectBtn);
        need_update = true;
    }
    if (need_update)
        update();

    return QDialog::eventFilter(watched, event);
}

BaseConfirmDlg &BaseConfirmDlg::setHeaderText(const QString &headerText)
{
    _ctrl->headerText.setValue(headerText);
    return *this;
}

BaseConfirmDlg &BaseConfirmDlg::setMessageText(const QString &messageText)
{
    _ctrl->messageText.setValue(messageText);
    return *this;
}

BaseConfirmDlg &BaseConfirmDlg::setAcceptButtonText(const QString &acceptText)
{
    _ctrl->acceptButtonText.setValue(acceptText);
    return *this;
}

BaseConfirmDlg &BaseConfirmDlg::setRejectButtonText(const QString &rejectText)
{
    _ctrl->rejectButtonText.setValue(rejectText);
    return *this;
}

BaseConfirmDlg &BaseConfirmDlg::setWarningIconVisible(bool visible)
{
    _ctrl->warningIconVisible.setValue(visible);
    return *this;
}

BaseConfirmDlg &BaseConfirmDlg::setWide(bool wide)
{
    _ctrl->isWide.setValue(wide);
    return *this;
}

BaseConfirmDlg &BaseConfirmDlg::setDefaultButton(DialogCode code)
{
    if (code == QDialog::Accepted) {
        _widgets.btnAccept->setDefault(true);
        _widgets.btnReject->setDefault(false);
    } else {
        _widgets.btnAccept->setDefault(false);
        _widgets.btnReject->setDefault(true);
    }
    return *this;
}

BaseConfirmDlg &BaseConfirmDlg::setSingleButton(bool val)
{
    _ctrl->singleButton.setValue(val);
    return *this;
}

void BaseConfirmDlg::setupCommonLogic(const CommonDialogWidgets& widgets)
{
    _widgets = widgets;

    DlgUtils::setTransparent(this);
    DlgUtils::applyDropShadowDialog(_widgets.frame);

    _widgets.btnAccept->setStyle(new FocusProxyStyle(_widgets.btnAccept));
    _widgets.btnReject->setStyle(new FocusProxyStyle(_widgets.btnReject));

    new WindowDragger(_widgets.frameHeader, this);

    auto updateThemeFunc = [this] {
        bool isDark = APP::Theme::instance()->isDarkTheme();
        applyTheme(isDark);
        DlgUtils::setTheme(this, isDark);
        if (_ctrl)
            _ctrl->darkTheme.setValue(isDark);
        update();
    };

    connect(APP::Theme::instance(), &APP::Theme::themeChanged, this, updateThemeFunc);
    updateThemeFunc();

    connect(_widgets.btnAccept, &QPushButton::clicked, this, &QDialog::accept);
    connect(_widgets.btnReject, &QPushButton::clicked, this, &QDialog::reject);

    auto syncUiFunc = [this]() {
        syncUI();
    };

    _notifiers.emplace_back(_ctrl->headerText.addNotifier(syncUiFunc));
    _notifiers.emplace_back(_ctrl->messageText.addNotifier(syncUiFunc));
    _notifiers.emplace_back(_ctrl->acceptButtonText.addNotifier(syncUiFunc));
    _notifiers.emplace_back(_ctrl->rejectButtonText.addNotifier(syncUiFunc));
    _notifiers.emplace_back(_ctrl->warningIconVisible.addNotifier(syncUiFunc));
    _notifiers.emplace_back(_ctrl->isWide.addNotifier(syncUiFunc));
    _notifiers.emplace_back(_ctrl->darkTheme.addNotifier(updateThemeFunc));
    _notifiers.emplace_back(_ctrl->singleButton.addNotifier(syncUiFunc));

    _widgets.btnAccept->installEventFilter(this);
    _widgets.btnReject->installEventFilter(this);
}

void BaseConfirmDlg::applyTheme(bool isDark)
{
}

void BaseConfirmDlg::syncUI()
{
    _widgets.btnReject->setVisible(!_ctrl->singleButton);
    if (_widgets.frameRejectBtn)
        _widgets.frameRejectBtn->setVisible(!_ctrl->singleButton);
    _widgets.lblHeader->setText(_ctrl->headerText);
    _widgets.lblText->setText(_ctrl->messageText);
    _widgets.btnAccept->setText(_ctrl->acceptButtonText);
    _widgets.btnReject->setText(_ctrl->rejectButtonText);

}
