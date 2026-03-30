#pragma once

#include <QDialog>

namespace Ui {class LogBrowser;}

class LogBrowser : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QString location READ location WRITE setLocation)
    Q_PROPERTY(bool enableLogging READ enableLogging WRITE setEnableLogging)
    Q_PROPERTY(bool enableHttpLogging READ enableHttpLogging WRITE setEnableHttpLogging)
    Q_PROPERTY(int filesToKeep READ filesToKeep WRITE setFilesToKeep)

signals:
    void openLocation();
    void logEnableChanged(bool enable);
    void logHttpEnableChanged(bool enable);
    void filesToKeepChanged(int count);

public:
    explicit LogBrowser(QWidget *parent = nullptr);
    ~LogBrowser();

    void setLocation(const QString& loc);
    QString location() const;

    void setEnableLogging(bool enable);
    bool enableLogging() const;

    void setEnableHttpLogging(bool enable);
    bool enableHttpLogging() const;

    void setFilesToKeep(int count);
    int filesToKeep() const;

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void themeChanged();

private:
    Ui::LogBrowser *ui = nullptr;
    QString _location;
};
