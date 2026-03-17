#include "custominputdlg.h"
#include "dlgutils.h"
#include "platform/common/baseinputdlg.h"

#ifdef Q_OS_WIN
#include "platform/windows/inputdlg.h"
#elif defined(Q_OS_MAC)
#include "platform/macos/inputdlgmac.h"
#endif

class CustomInputDlgPrivate
{
public:
    CustomInputDlgPrivate(QWidget *p)
        : parent(p)
    {
    }

    BaseInputDlg *dlg = nullptr;

    QString headerText;
    QString promptText;
    QString acceptText;
    QString rejectText;
    QString inputText;

    QWidget *parent = nullptr;
    bool deleteOnClose = false;

    QDialog::DialogCode defaultCode = QDialog::Accepted;

    void ensureCreated(APP::CustomInputDlg* q)
    {
        static bool resourcesLoaded = []() {
            Q_INIT_RESOURCE(customdialogs_res);
            return true;
        }();

        if (dlg)
            return;

#ifdef Q_OS_WIN
        dlg = new InputDlg(parent);
#elif defined(Q_OS_MAC)
        dlg = new InputDlgMac(nullptr);
#endif

        dlg->setHeaderText(headerText);
        dlg->setPromptText(promptText);
        dlg->setAcceptButtonText(acceptText);
        dlg->setRejectButtonText(rejectText);
        dlg->setDefaultButton(defaultCode);

        QObject::connect(dlg, &BaseInputDlg::accepted, q, &APP::CustomInputDlg::accepted);
        QObject::connect(dlg, &BaseInputDlg::rejected, q, &APP::CustomInputDlg::rejected);
        QObject::connect(dlg, &BaseInputDlg::finished, q, &APP::CustomInputDlg::finished);
        QObject::connect(dlg, &BaseInputDlg::inputTextChanged, [this](const QString& text) {
            inputText = text;
        });

        if (deleteOnClose) {
            QObject::connect(dlg, &QDialog::finished, q, &QObject::deleteLater);
        }

        DlgUtils::centerDialog(parent, dlg);
    }

};


namespace APP {

CustomInputDlg::CustomInputDlg(QWidget *parent)
    : d_ptr(new CustomInputDlgPrivate(parent))
{

}

CustomInputDlg::~CustomInputDlg()
{
}

int CustomInputDlg::exec()
{
    Q_D(CustomInputDlg);
    d->ensureCreated(this);
    return d->dlg->exec();
}

CustomInputDlg &CustomInputDlg::setHeaderText(const QString &headerText)
{
    Q_D(CustomInputDlg);
    d->headerText = headerText;
    return *this;
}

CustomInputDlg &CustomInputDlg::setPromptText(const QString &promptText)
{
    Q_D(CustomInputDlg);
    d->promptText = promptText;
    return *this;
}

CustomInputDlg &CustomInputDlg::setAcceptButtonText(const QString &acceptText)
{
    Q_D(CustomInputDlg);
    d->acceptText = acceptText;
    return *this;
}

CustomInputDlg &CustomInputDlg::setRejectButtonText(const QString &rejectText)
{
    Q_D(CustomInputDlg);
    d->rejectText = rejectText;
    return *this;
}

QString CustomInputDlg::inputText() const
{
    Q_D(const CustomInputDlg);
    return d->inputText;
}

QWidget *CustomInputDlg::widgetPtr() const
{
    Q_D(const CustomInputDlg);
    return d->dlg;
}

void CustomInputDlg::accept()
{
    Q_D(CustomInputDlg);
    d->inputText = d->dlg->inputText();
    d->dlg->accept();
}

void CustomInputDlg::reject()
{
    Q_D(const CustomInputDlg);
    d->dlg->reject();
}

} // namespace APP
