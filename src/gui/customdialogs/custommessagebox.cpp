#include "custommessagebox.h"

#include "dlgutils.h"

#include "gui/application.h"
#include "gui/settingsdialog.h"

#include <QPointer>
#include <QtGlobal>

#ifdef Q_OS_WIN
#    include "platform/windows/confirmdlg.h"
#elif defined(Q_OS_MAC)
#    include "platform/macos/confirmdlgmac.h"
#    include "platform/macos/confirmwidedlgmac.h"
#endif

class CustomMessageBoxPrivate
{
public:
    CustomMessageBoxPrivate(QWidget *p)
        : parent(p)
    {
        const auto mainParent = APP::ocApp()->gui()->settingsDialog();
        if (mainParent)
            parent = mainParent;
    }

    ~CustomMessageBoxPrivate()
    {
        delete dlg;
    }

    QPointer<BaseConfirmDlg> dlg;

    QString headerText;
    QString messageText;
    QString acceptText;
    QString rejectText;
    bool isWarning = false;
    bool isWide = false;
    QWidget *parent = nullptr;
    bool deleteOnClose = false;
    bool singleButton = false;
    QDialog::DialogCode defaultCode = QDialog::Accepted;

    void ensureCreated(APP::CustomMessageBox *q)
    {
        static bool resourcesLoaded = []() {
            Q_INIT_RESOURCE(customdialogs_res);
            return true;
        }();

        if (dlg)
            return;

#ifdef Q_OS_WIN
        auto winDlg = new ConfirmDlg(parent);
        winDlg->setWide(isWide);
        dlg = winDlg;
#elif defined(Q_OS_MAC)
        if (isWide)
            dlg = new ConfirmWideDlgMac(nullptr);
        else
            dlg = new ConfirmDlgMac(nullptr);
#else
#    error "CustomMessageBox: unsupported platform (only Windows and macOS are supported)"
#endif
        dlg->setRealParent(parent);
        dlg->setHeaderText(headerText);
        dlg->setMessageText(messageText);
        dlg->setAcceptButtonText(acceptText);
        dlg->setRejectButtonText(rejectText);
        dlg->setWarningIconVisible(isWarning);
        dlg->setDefaultButton(defaultCode);
        dlg->setSingleButton(singleButton);

        QObject::connect(dlg, &BaseConfirmDlg::accepted, q, &APP::CustomMessageBox::accepted);
        QObject::connect(dlg, &BaseConfirmDlg::rejected, q, &APP::CustomMessageBox::rejected);
        QObject::connect(dlg, &BaseConfirmDlg::finished, q, &APP::CustomMessageBox::finished);

        if (deleteOnClose) {
            QObject::connect(dlg, &QDialog::finished, q, &QObject::deleteLater);
        }
    }
};

namespace APP
{

CustomMessageBox::CustomMessageBox(QWidget *parent)
    : d_ptr(new CustomMessageBoxPrivate(parent))
{
}

CustomMessageBox::~CustomMessageBox()
{
}

int CustomMessageBox::exec()
{
    Q_D(CustomMessageBox);
    d->ensureCreated(this);
    return d->dlg->exec();
}

void CustomMessageBox::show()
{
    Q_D(CustomMessageBox);
    d->ensureCreated(this);
#ifdef Q_OS_MAC
    d->dlg->setAttribute(Qt::WA_MacAlwaysShowToolWindow, true);
    d->dlg->setWindowModality(Qt::ApplicationModal);
    d->dlg->setResult(0);
    //DlgUtils::centerDialog(d->parent, d->dlg);
#endif
    d->dlg->show();
}

void CustomMessageBox::open()
{
    Q_D(CustomMessageBox);
    d->ensureCreated(this);
#ifdef Q_OS_MAC
    d->dlg->setAttribute(Qt::WA_MacAlwaysShowToolWindow, true);
    d->dlg->setWindowModality(Qt::ApplicationModal);
    d->dlg->setResult(0);
    //DlgUtils::centerDialog(d->parent, d->dlg);
    d->dlg->show();
#else
    d->dlg->open();
#endif
}

CustomMessageBox &CustomMessageBox::setHeaderText(const QString &headerText)
{
    Q_D(CustomMessageBox);
    d->headerText = headerText;
    return *this;
}

CustomMessageBox &CustomMessageBox::setMessageText(const QString &messageText)
{
    Q_D(CustomMessageBox);
    d->messageText = messageText;
    return *this;
}

CustomMessageBox &CustomMessageBox::setAcceptButtonText(const QString &acceptText)
{
    Q_D(CustomMessageBox);
    d->acceptText = acceptText;
    return *this;
}

CustomMessageBox &CustomMessageBox::setRejectButtonText(const QString &rejectText)
{
    Q_D(CustomMessageBox);
    d->rejectText = rejectText;
    return *this;
}

CustomMessageBox &CustomMessageBox::setWarningIconVisible(bool visible)
{
    Q_D(CustomMessageBox);
    d->isWarning = visible;
    return *this;
}

CustomMessageBox &CustomMessageBox::setWide(bool wide)
{
    Q_D(CustomMessageBox);
    d->isWide = wide;
    return *this;
}

CustomMessageBox &CustomMessageBox::setDefaultButton(QDialog::DialogCode code)
{
    Q_D(CustomMessageBox);
    d->defaultCode = code;
    return *this;
}

CustomMessageBox &CustomMessageBox::setSingleButton(bool val)
{
    Q_D(CustomMessageBox);
    d->singleButton = val;
    return *this;
}

CustomMessageBox &CustomMessageBox::setSingleButtonText(const QString &text)
{
    Q_D(CustomMessageBox);
    d->acceptText = text;
    return *this;
}

CustomMessageBox &CustomMessageBox::setDeleteOnClose(bool on)
{
    Q_D(CustomMessageBox);
    d->deleteOnClose = on;
    if (d->dlg) {
        if (on) {
            connect(d->dlg, &QDialog::finished, this, &QObject::deleteLater);
        }
        else {
            disconnect(d->dlg, &QDialog::finished, this, &QObject::deleteLater);
        }
    }
    return *this;
}

QWidget *CustomMessageBox::widgetPtr() const
{
    Q_D(const CustomMessageBox);
    return d->dlg.data();
}

void CustomMessageBox::accept()
{
    Q_D(const CustomMessageBox);
    if (!d->dlg)
        return;
    d->dlg->accept();
}

void CustomMessageBox::reject()
{
    Q_D(const CustomMessageBox);
    if (!d->dlg)
        return;
    d->dlg->reject();
}

}   // namespace APP
