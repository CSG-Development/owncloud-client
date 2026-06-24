#include "baseinputdlg.h"

#include "dlgutils.h"
#include "prophelper.h"
#include "pushbuttonstyle.h"
#include "theme.h"
#include "windowdragger.h"

#include "gui/customui/focusproxy.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>

namespace
{
const auto logo_icon = QStringLiteral(":/res/Files-app-icon-round.svg");
}

BaseInputDlg::BaseInputDlg(QWidget *parent)
    : QDialog(parent)
    , _ctrl(new InputDlgController(this))
{
}

BaseInputDlg &BaseInputDlg::setPromptText(const QString &promptText)
{
    _ctrl->promptText.setValue(promptText);
    return *this;
}

BaseInputDlg &BaseInputDlg::setAcceptButtonText(const QString &acceptText)
{
    _ctrl->acceptButtonText.setValue(acceptText);
    return *this;
}

BaseInputDlg &BaseInputDlg::setRejectButtonText(const QString &rejectText)
{
    _ctrl->rejectButtonText.setValue(rejectText);
    return *this;
}

BaseInputDlg &BaseInputDlg::setDefaultButton(DialogCode code)
{
    if (!_widgets.btnAccept || !_widgets.btnReject)
        return *this;

    if (code == QDialog::Accepted) {
        _widgets.btnAccept->setDefault(true);
        _widgets.btnReject->setDefault(false);
    }
    else {
        _widgets.btnAccept->setDefault(false);
        _widgets.btnReject->setDefault(true);
    }
    return *this;
}

QString BaseInputDlg::inputText() const
{
    return _ctrl->inputText.value();
}

BaseInputDlg &BaseInputDlg::setHeaderText(const QString &headerText)
{
    _ctrl->headerText.setValue(headerText);
    return *this;
}

bool BaseInputDlg::eventFilter(QObject *watched, QEvent *event)
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

void BaseInputDlg::setupCommonLogic(const CommonDialogWidgets &widgets)
{
    _widgets = widgets;

    DlgUtils::setTransparent(this);
    DlgUtils::applyDropShadowDialog(_widgets.frame);
    _widgets.btnAccept->setStyle(new FocusProxyStyle(_widgets.btnAccept));
    _widgets.btnReject->setStyle(new FocusProxyStyle(_widgets.btnReject));

    new WindowDragger(_widgets.frameHeader, this);

    auto updateThemeFunc = [this] {
        bool isDark = APP::Theme::instance()->isDarkTheme();
        DlgUtils::setTheme(this, isDark);
        applyTheme(isDark);
        if (_ctrl)
            _ctrl->darkTheme.setValue(isDark);
        update();
    };

    connect(APP::Theme::instance(), &APP::Theme::themeChanged, this, updateThemeFunc);
    updateThemeFunc();

    if (_widgets.btnIcon)
        _widgets.btnIcon->setIcon(QIcon(logo_icon));

    connect(_widgets.btnAccept, &QPushButton::clicked, this, &QDialog::accept);
    connect(_widgets.btnReject, &QPushButton::clicked, this, &QDialog::reject);

    if (_widgets.edText) {
        connect(_widgets.edText, &QLineEdit::textChanged, this, [this](const QString &text) {
            _ctrl->inputText.setValue(text);
            emit inputTextChanged(text);
        });
    }

    auto syncUiFunc = [this]() {
        syncUI();
    };

    _notifiers.emplace_back(_ctrl->headerText.addNotifier(syncUiFunc));
    _notifiers.emplace_back(_ctrl->promptText.addNotifier(syncUiFunc));
    _notifiers.emplace_back(_ctrl->acceptButtonText.addNotifier(syncUiFunc));
    _notifiers.emplace_back(_ctrl->rejectButtonText.addNotifier(syncUiFunc));
    _notifiers.emplace_back(_ctrl->darkTheme.addNotifier(updateThemeFunc));

    _widgets.btnAccept->installEventFilter(this);
    _widgets.btnReject->installEventFilter(this);
}

void BaseInputDlg::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (_isFirstShow) {
        _isFirstShow = false;
        adjustSize();

        if (_realParent) {
            QPoint parentCenter = _realParent->mapToGlobal(_realParent->rect().center());
            move(parentCenter.x() - width() / 2, parentCenter.y() - height() / 2);
        }
        else {
            QScreen *screen = QGuiApplication::primaryScreen();
            if (screen) {
                QRect screenGeometry = screen->geometry();
                move(screenGeometry.center().x() - width() / 2,
                     screenGeometry.center().y() - height() / 2);
            }
        }
    }
}

void BaseInputDlg::applyTheme(bool /*isDark*/)
{
}

void BaseInputDlg::syncUI()
{
    _widgets.lblHeader->setText(_ctrl->headerText);
    _widgets.lblText->setText(_ctrl->promptText);
    _widgets.btnAccept->setText(_ctrl->acceptButtonText);
    _widgets.btnReject->setText(_ctrl->rejectButtonText);
}
