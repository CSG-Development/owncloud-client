#include "onboardingstate.h"

#include "configfile.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStringList>
#include <QtGlobal>

#include <memory>

namespace {
const auto onboardingGroupC = QStringLiteral("Onboarding");
const auto lastShownPageC = QStringLiteral("lastShownPage");
const auto completedC = QStringLiteral("completed");
const auto dismissedC = QStringLiteral("dismissed");
const auto installerMetaFileNameC = QStringLiteral("PersonalCloud.installer-meta.ini");
const auto onboardingRequiredC = QStringLiteral("onboarding_required");
#ifdef Q_OS_MACOS
const auto macOSInstallerMetaDirC = QStringLiteral("Library/Application Support/Personal Cloud Files");
#endif

std::unique_ptr<QSettings> onboardingSettings()
{
    return APP::ConfigFile::settingsWithGroup(onboardingGroupC);
}

QString installerMetaFilePath()
{
    const QDir appDir(QCoreApplication::applicationDirPath());
    QStringList candidates {
        appDir.filePath(installerMetaFileNameC),
        appDir.filePath(QStringLiteral("../../../") + installerMetaFileNameC),
    };
#ifdef Q_OS_MACOS
    candidates << QDir(QDir::homePath()).filePath(
        macOSInstallerMetaDirC + QLatin1Char('/') + installerMetaFileNameC);
#endif

    for (const auto &candidate : candidates) {
        const QFileInfo fileInfo(candidate);
        if (fileInfo.isFile()) {
            return fileInfo.absoluteFilePath();
        }
    }

    return {};
}
}

namespace APP {

bool OnboardingState::shouldShow() const
{
    if (!installerRequiresOnboarding()) {
        return false;
    }

    auto settings = onboardingSettings();
    if (settings->value(completedC, false).toBool() || settings->value(dismissedC, false).toBool()) {
        return false;
    }

    return nextPage() < PageCount;
}

int OnboardingState::nextPage() const
{
    auto settings = onboardingSettings();
    const int lastShownPage = settings->value(lastShownPageC, -1).toInt();
    return qBound(0, lastShownPage + 1, PageCount - 1);
}

void OnboardingState::markPageShown(int page)
{
    const int normalizedPage = qBound(0, page, PageCount - 1);
    auto settings = onboardingSettings();
    const int lastShownPage = settings->value(lastShownPageC, -1).toInt();
    if (normalizedPage > lastShownPage) {
        settings->setValue(lastShownPageC, normalizedPage);
    }
    if (normalizedPage == PageCount - 1) {
        settings->setValue(completedC, true);
    }
    settings->sync();
}

void OnboardingState::markCompleted()
{
    auto settings = onboardingSettings();
    settings->setValue(lastShownPageC, PageCount - 1);
    settings->setValue(completedC, true);
    settings->sync();
}

void OnboardingState::markDismissed()
{
    auto settings = onboardingSettings();
    settings->setValue(dismissedC, true);
    settings->sync();
}

bool OnboardingState::installerRequiresOnboarding() const
{
    const QString metaFile = installerMetaFilePath();
    if (metaFile.isEmpty()) {
        return false;
    }

    const QSettings settings(metaFile, QSettings::IniFormat);
    return settings.value(onboardingRequiredC, false).toBool();
}

} // namespace APP
