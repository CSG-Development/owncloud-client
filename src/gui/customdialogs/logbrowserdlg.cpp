#include "logbrowserdlg.h"

#include "configfile.h"
#include "logger.h"

#include "gui/customdialogs/dlgutils.h"
#include "gui/customdialogs/platform/common/prophelper.h"

#include <QDesktopServices>
#include <QDir>
#include <QPointer>

#ifdef Q_OS_WIN
#    include "platform/windows/logbrowser.h"

#elif defined(Q_OS_MAC)
#    include "platform/macos/logbrowsermac.h"
#endif

class LogBrowserDlgPrivate
{
public:
    LogBrowserDlgPrivate(QWidget *p)
        : parent(p)
    {
    }

    ~LogBrowserDlgPrivate()
    {
        delete dlg;
    }

    QPointer<QDialog> dlg;
    QWidget *parent = nullptr;
    bool deleteOnClose = false;
    QDialog::DialogCode defaultCode = QDialog::Accepted;
    QString locationStr;
    bool logEnabled = true;
    bool logHttpEnabled = false;
    int filesToKeep = 5;

    void ensureCreated(APP::LogBrowserDlg *q)
    {
        static bool resourcesLoaded = []() {
            Q_INIT_RESOURCE(customdialogs_res);
            return true;
        }();

        if (dlg)
            return;

#ifdef Q_OS_WIN
        dlg = new LogBrowser(parent);
        QObject::connect(qobject_cast<LogBrowser *>(dlg), &LogBrowser::openLocation, [q] {
            if (q)
                q->openLocation();
        });
        QObject::connect(qobject_cast<LogBrowser *>(dlg), &LogBrowser::logEnableChanged, [q](bool enable) {
            if (q)
                q->logEnable(enable);
        });
        QObject::connect(qobject_cast<LogBrowser *>(dlg), &LogBrowser::logHttpEnableChanged, [q](bool enable) {
            if (q)
                q->logHttpEnable(enable);
        });
        QObject::connect(qobject_cast<LogBrowser *>(dlg), &LogBrowser::filesToKeepChanged, [q](int count) {
            if (q)
                q->filesToKeep(count);
        });
#elif defined(Q_OS_MAC)
        dlg = new LogBrowserMac(nullptr);
        QObject::connect(qobject_cast<LogBrowserMac *>(dlg), &LogBrowserMac::openLocation, [q] {
            if (q)
                q->openLocation();
        });
        QObject::connect(qobject_cast<LogBrowserMac *>(dlg), &LogBrowserMac::logEnableChanged, [q](bool enable) {
            if (q)
                q->logEnable(enable);
        });
        QObject::connect(qobject_cast<LogBrowserMac *>(dlg), &LogBrowserMac::logHttpEnableChanged, [q](bool enable) {
            if (q)
                q->logHttpEnable(enable);
        });
        QObject::connect(qobject_cast<LogBrowserMac *>(dlg), &LogBrowserMac::filesToKeepChanged, [q](int count) {
            if (q)
                q->filesToKeep(count);
        });
#else
#    error "LogBrowserDlg: unsupported platform (only Windows and macOS are supported)"
#endif

        QObject::connect(dlg, &QDialog::accepted, q, &APP::LogBrowserDlg::accepted);
        QObject::connect(dlg, &QDialog::rejected, q, &APP::LogBrowserDlg::rejected);
        QObject::connect(dlg, &QDialog::finished, q, &APP::LogBrowserDlg::finished);

        safeSetProperty(dlg, "location", locationStr);
        safeSetProperty(dlg, "enableLogging", logEnabled);
        safeSetProperty(dlg, "enableHttpLogging", logHttpEnabled);
        safeSetProperty(dlg, "filesToKeep", filesToKeep);

        if (deleteOnClose) {
            QObject::connect(dlg, &QDialog::finished, q, &QObject::deleteLater);
        }
    }
};

namespace APP
{

LogBrowserDlg::LogBrowserDlg(QWidget *parent)
    : d_ptr(new LogBrowserDlgPrivate(parent))
{
    setLocation(Logger::instance()->temporaryFolderLogDirPath());
    setEnableLogging(ConfigFile().automaticLogDir());
    setEnableHttpLogging(ConfigFile().logHttp());
    setFilesToKeep(ConfigFile().automaticDeleteOldLogs());
}

LogBrowserDlg::~LogBrowserDlg()
{
}

void LogBrowserDlg::open()
{
    Q_D(LogBrowserDlg);
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

void LogBrowserDlg::accept()
{
    Q_D(LogBrowserDlg);
    if (!d->dlg)
        return;
    d->dlg->accept();
}

void LogBrowserDlg::reject()
{
    Q_D(const LogBrowserDlg);
    if (!d->dlg)
        return;
    d->dlg->reject();
}

void LogBrowserDlg::openLocation()
{
    QString path = Logger::instance()->temporaryFolderLogDirPath();
    QDir().mkpath(path);
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void LogBrowserDlg::logEnable(bool enable)
{
    ConfigFile config;
    config.setAutomaticLogDir(enable);
    setupLoggingFromConfig();
}

void LogBrowserDlg::logHttpEnable(bool enable)
{
    ConfigFile().configureHttpLogging(std::make_optional(enable));
}

void LogBrowserDlg::filesToKeep(int count)
{
    ConfigFile().setAutomaticDeleteOldLogs(count);
    Logger::instance()->setMaxLogFiles(count);
}

LogBrowserDlg &LogBrowserDlg::setDeleteOnClose(bool on)
{
    Q_D(LogBrowserDlg);
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

LogBrowserDlg &LogBrowserDlg::setLocation(const QString &text)
{
    Q_D(LogBrowserDlg);
    d->locationStr = text;
    return *this;
}

LogBrowserDlg &LogBrowserDlg::setEnableLogging(bool enable)
{
    Q_D(LogBrowserDlg);
    d->logEnabled = enable;
    return *this;
}

LogBrowserDlg &LogBrowserDlg::setEnableHttpLogging(bool enable)
{
    Q_D(LogBrowserDlg);
    d->logHttpEnabled = enable;
    return *this;
}

LogBrowserDlg &LogBrowserDlg::setFilesToKeep(int count)
{
    Q_D(LogBrowserDlg);
    d->filesToKeep = count;
    return *this;
}

void LogBrowserDlg::setupLoggingFromConfig()
{
    ConfigFile config;
    auto logger = Logger::instance();

    if (config.automaticLogDir()) {
        // Don't override other configured logging
        if (logger->isLoggingToFile())
            return;

        logger->setupTemporaryFolderLogDir();
        Logger::instance()->setMaxLogFiles(config.automaticDeleteOldLogs());
    }
    else {
        logger->disableTemporaryFolderLogDir();
    }
}

}   // namespace APP
