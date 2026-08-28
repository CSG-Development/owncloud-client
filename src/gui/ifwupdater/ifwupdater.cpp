#include "ifwupdater.h"

#include "config.h"
#include "configfile.h"

#include <QCoreApplication>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

using namespace std::chrono_literals;

namespace APP {

Q_LOGGING_CATEGORY(lcIfwUpdater, "gui.ifwupdater", QtInfoMsg)

namespace {

QString maintenanceStateDir()
{
#if defined(Q_OS_WIN)
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#else
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
#endif
    return base + QStringLiteral("/maintenancetool");
}

} // namespace

IFWUpdater::IFWUpdater(QObject *parent)
    : QObject(parent)
{
}

QString IFWUpdater::maintenanceToolPath() const
{
#ifdef APPLICATION_MAINTENANCETOOL_EXECUTABLE
    const QString name = QStringLiteral(APPLICATION_MAINTENANCETOOL_EXECUTABLE);
#else
    const QString name = QStringLiteral("MaintenanceTool");
#endif

#if defined(Q_OS_WIN)
    return QDir(QCoreApplication::applicationDirPath()).filePath(name + QStringLiteral(".exe"));
#elif defined(Q_OS_MAC)
    return QStringLiteral("/Applications/%1.app/Contents/MacOS/%1").arg(name);
#else
    return QDir(QCoreApplication::applicationDirPath()).filePath(name);
#endif
}

QStringList IFWUpdater::checkUpdatesArguments() const
{
    return { QStringLiteral("--checkupdates") };
}

void IFWUpdater::syncRepositoryOverride()
{
    const QString url = ConfigFile().updateRepositoryUrlOverride();
    const QString dir = maintenanceStateDir();
    QDir().mkpath(dir);
    const QString path = QDir(dir).filePath(QStringLiteral("network.xml"));

    QDomDocument doc;
    QFile inFile(path);
    if (inFile.open(QIODevice::ReadOnly)) {
        doc.setContent(&inFile);
        inFile.close();
    }

    QDomElement root = doc.firstChildElement(QStringLiteral("Network"));
    if (root.isNull()) {
        doc = QDomDocument();
        root = doc.createElement(QStringLiteral("Network"));
        doc.appendChild(root);
    }

    QDomElement repositories = root.firstChildElement(QStringLiteral("Repositories"));
    if (!repositories.isNull())
        root.removeChild(repositories);

    repositories = doc.createElement(QStringLiteral("Repositories"));
    if (!url.isEmpty()) {
        QDomElement repo = doc.createElement(QStringLiteral("Repository"));
        const auto addText = [&doc, &repo](const QString &tag, const QString &value) {
            QDomElement el = doc.createElement(tag);
            el.appendChild(doc.createTextNode(value));
            repo.appendChild(el);
        };
        addText(QStringLiteral("Host"), url);
        addText(QStringLiteral("Username"), QString());
        addText(QStringLiteral("Password"), QString());
        addText(QStringLiteral("Enabled"), QStringLiteral("1"));
        repositories.appendChild(repo);
    }
    root.appendChild(repositories);

    QFile outFile(path);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCWarning(lcIfwUpdater) << "Cannot write" << path << outFile.errorString();
        return;
    }
    outFile.write(doc.toByteArray(1));
}

void IFWUpdater::checkForUpdate()
{
    if (_process) {
        qCInfo(lcIfwUpdater) << "Update check already in progress";
        return;
    }

    const QString tool = maintenanceToolPath();
    if (!QFileInfo::exists(tool)) {
        qCWarning(lcIfwUpdater) << "Maintenance tool not found at" << tool;
        emit checkFailed(QStringLiteral("Maintenance tool not found"));
        return;
    }

    _process = new QProcess(this);
    connect(_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, &IFWUpdater::onCheckUpdatesFinished);

    qCInfo(lcIfwUpdater) << "Running" << tool << checkUpdatesArguments();
    _process->start(tool, checkUpdatesArguments());
}

void IFWUpdater::onCheckUpdatesFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    const QByteArray output = _process->readAllStandardOutput();
    _process->deleteLater();
    _process = nullptr;

    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        qCWarning(lcIfwUpdater) << "checkupdates failed, exit code" << exitCode;
        emit checkFailed(QStringLiteral("checkupdates failed"));
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(output)) {
        qCInfo(lcIfwUpdater) << "No updates available (non-XML response)";
        emit noUpdateAvailable();
        return;
    }

    const QDomNodeList updates = doc.elementsByTagName(QStringLiteral("update"));
    if (updates.isEmpty()) {
        qCInfo(lcIfwUpdater) << "No updates available";
        emit noUpdateAvailable();
        return;
    }

    for (int i = 0; i < updates.count(); ++i) {
        const QDomElement el = updates.at(i).toElement();
        qCInfo(lcIfwUpdater) << "Update available:" << el.attribute(QStringLiteral("name")) << el.attribute(QStringLiteral("version"));
    }

    const QDomElement first = updates.at(0).toElement();
    emit updateAvailable(first.attribute(QStringLiteral("name")), first.attribute(QStringLiteral("version")));
}

IFWUpdaterScheduler::IFWUpdaterScheduler(QObject *parent)
    : QObject(parent)
{
    connect(&_updateCheckTimer, &QTimer::timeout, this, &IFWUpdaterScheduler::slotTimerFired);
    connect(&_updater, &IFWUpdater::updateAvailable, this, [this](const QString &name, const QString &version) {
        emit updaterAnnouncement(tr("Update Check"), tr("%1 %2 is available.").arg(name, version));
    });

    QTimer::singleShot(3s, this, &IFWUpdaterScheduler::slotTimerFired);

    ConfigFile cfg;
    _updateCheckTimer.start(std::chrono::milliseconds(cfg.updateCheckInterval()).count());
}

void IFWUpdaterScheduler::slotTimerFired()
{
    ConfigFile cfg;

    const auto checkInterval = std::chrono::milliseconds(cfg.updateCheckInterval()).count();
    if (checkInterval != _updateCheckTimer.interval()) {
        _updateCheckTimer.setInterval(checkInterval);
        qCInfo(lcIfwUpdater) << "Setting new update check interval" << checkInterval;
    }

    if (cfg.skipUpdateCheck()) {
        qCInfo(lcIfwUpdater) << "Skipping update check because of config file";
        return;
    }

    _updater.checkForUpdate();
}

} // namespace APP
