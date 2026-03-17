#pragma once

#include <QDialog>

class LogBrowserDlgPrivate;

namespace APP {

class LogBrowserDlg: public QObject
{
    Q_OBJECT

public:
    explicit LogBrowserDlg(QWidget *parent = nullptr);
    ~LogBrowserDlg();

    void open();

    LogBrowserDlg& setDeleteOnClose(bool on = true);
    LogBrowserDlg& setLocation(const QString& text);
    LogBrowserDlg& setEnableLogging(bool enable);
    LogBrowserDlg& setEnableHttpLogging(bool enable);
    LogBrowserDlg& setFilesToKeep(int count);

    static void setupLoggingFromConfig();

public slots:
    void accept();
    void reject();
    void openLocation();
    void logEnable(bool enable);
    void logHttpEnable(bool enable);
    void filesToKeep(int count);

signals:
    void accepted();
    void rejected();
    void finished(int result);

private:
    Q_DECLARE_PRIVATE(LogBrowserDlg)
    QScopedPointer<LogBrowserDlgPrivate> d_ptr;
};

} // namespace APP
