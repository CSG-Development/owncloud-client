#ifndef IFWUPDATER_H
#define IFWUPDATER_H

#include <QLoggingCategory>
#include <QObject>
#include <QProcess>
#include <QTimer>

namespace APP {

Q_DECLARE_LOGGING_CATEGORY(lcIfwUpdater)

class IFWUpdater : public QObject
{
    Q_OBJECT
public:
    explicit IFWUpdater(QObject *parent = nullptr);

    void checkForUpdate();

    static void syncRepositoryOverride();

signals:
    void updateAvailable(const QString &displayName, const QString &version);
    void noUpdateAvailable();
    void checkFailed(const QString &reason);

private slots:
    void onCheckUpdatesFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QString maintenanceToolPath() const;
    QStringList checkUpdatesArguments() const;

    QProcess *_process = nullptr;
};

class IFWUpdaterScheduler : public QObject
{
    Q_OBJECT
public:
    explicit IFWUpdaterScheduler(QObject *parent = nullptr);

signals:
    void updaterAnnouncement(const QString &title, const QString &msg);

private slots:
    void slotTimerFired();

private:
    QTimer _updateCheckTimer;
    IFWUpdater _updater;
};

} // namespace APP

#endif // IFWUPDATER_H
