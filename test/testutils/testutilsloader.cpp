#include "configfile.h"
#include "logger.h"
#include "resources/loadresources.h"
#include "testutils.h"

#include <QCoreApplication>

namespace {
void setUpTests()
{
    // load the resources
    static const APP::ResourcesLoader resources;

    static auto dir = APP::TestUtils::createTempDir();
    APP::ConfigFile::setConfDir(QStringLiteral("%1/config").arg(dir.path())); // we don't want to pollute the user's config file

    APP::Logger::instance()->setLogFile(QStringLiteral("-"));
    APP::Logger::instance()->addLogRule({ QStringLiteral("sync.httplogger=true") });
    APP::Logger::instance()->setLogDebug(true);

    APP::Account::setCommonCacheDirectory(QStringLiteral("%1/cache").arg(dir.path()));
}
Q_COREAPP_STARTUP_FUNCTION(setUpTests)
}
