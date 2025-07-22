#include "configfile.h"
#include "logger.h"
#include "resources/loadresources.h"
#include "testutils.h"

#include <QCoreApplication>

namespace {
void setUpTests()
{
    // load the resources
    static const CUR::ResourcesLoader resources;

    static auto dir = CUR::TestUtils::createTempDir();
    CUR::ConfigFile::setConfDir(QStringLiteral("%1/config").arg(dir.path())); // we don't want to pollute the user's config file

    CUR::Logger::instance()->setLogFile(QStringLiteral("-"));
    CUR::Logger::instance()->addLogRule({ QStringLiteral("sync.httplogger=true") });
    CUR::Logger::instance()->setLogDebug(true);

    CUR::Account::setCommonCacheDirectory(QStringLiteral("%1/cache").arg(dir.path()));
}
Q_COREAPP_STARTUP_FUNCTION(setUpTests)
}
