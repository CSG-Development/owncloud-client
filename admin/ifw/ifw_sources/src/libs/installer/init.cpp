/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#include "init.h"

#include "createshortcutoperation.h"
#include "createdesktopentryoperation.h"
#include "createlocalrepositoryoperation.h"
#include "extractarchiveoperation.h"
#include "globalsettingsoperation.h"
#include "environmentvariablesoperation.h"
#include "registerfiletypeoperation.h"
#include "selfrestartoperation.h"
#include "installiconsoperation.h"
#include "elevatedexecuteoperation.h"
#include "fakestopprocessforupdateoperation.h"
#include "stopprocessforupdateoperation.h"
#include "createlinkoperation.h"
#include "simplemovefileoperation.h"
#include "copydirectoryoperation.h"
#include "replaceoperation.h"
#include "linereplaceoperation.h"
#include "minimumprogressoperation.h"
#include "licenseoperation.h"
#include "settingsoperation.h"
#include "consumeoutputoperation.h"
#include "loggingutils.h"

#include "updateoperationfactory.h"
#include "filedownloaderfactory.h"

#include <QtPlugin>

using namespace KDUpdater;
using namespace QInstaller;

#if defined(QT_STATIC)
static void initResources()
{
    Q_INIT_RESOURCE(installer);
}
#endif

/*!
    Initializes the installer resources. Registers
    custom operations and installs a custom message handler.
*/
void QInstaller::init()
{
#if defined(QT_STATIC)
    ::initResources();
#endif

    UpdateOperationFactory &factory = UpdateOperationFactory::instance();
    factory.registerUpdateOperation<CreateShortcutOperation>(QLatin1String("CreateShortcut"));
    factory.registerUpdateOperation<CreateDesktopEntryOperation>(QLatin1String("CreateDesktopEntry"));
    factory.registerUpdateOperation<CreateLocalRepositoryOperation>(QLatin1String("CreateLocalRepository"));
    factory.registerUpdateOperation<ExtractArchiveOperation>(QLatin1String("Extract"));
    factory.registerUpdateOperation<GlobalSettingsOperation>(QLatin1String("GlobalConfig"));
    factory.registerUpdateOperation<EnvironmentVariableOperation>(QLatin1String("EnvironmentVariable"));
    factory.registerUpdateOperation<RegisterFileTypeOperation>(QLatin1String("RegisterFileType"));
    factory.registerUpdateOperation<SelfRestartOperation>(QLatin1String("SelfRestart"));
    factory.registerUpdateOperation<InstallIconsOperation>(QLatin1String("InstallIcons"));
    factory.registerUpdateOperation<ElevatedExecuteOperation>(QLatin1String("Execute"));
    factory.registerUpdateOperation<FakeStopProcessForUpdateOperation>(QLatin1String("FakeStopProcessForUpdate"));
    factory.registerUpdateOperation<StopProcessForUpdateOperation>(QLatin1String("StopProcessForUpdate"));
    factory.registerUpdateOperation<CreateLinkOperation>(QLatin1String("CreateLink"));
    factory.registerUpdateOperation<SimpleMoveFileOperation>(QLatin1String("SimpleMoveFile"));
    factory.registerUpdateOperation<CopyDirectoryOperation>(QLatin1String("CopyDirectory"));
    factory.registerUpdateOperation<ReplaceOperation>(QLatin1String("Replace"));
    factory.registerUpdateOperation<LineReplaceOperation>(QLatin1String("LineReplace"));
    factory.registerUpdateOperation<MinimumProgressOperation>(QLatin1String("MinimumProgress"));
    factory.registerUpdateOperation<LicenseOperation>(QLatin1String("License"));
    factory.registerUpdateOperation<ConsumeOutputOperation>(QLatin1String("ConsumeOutput"));
    factory.registerUpdateOperation<SettingsOperation>(QLatin1String("Settings"));

    auto messageHandler = [](QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        LoggingHandler::instance().messageHandler(type, context, msg);
    };
    qInstallMessageHandler(messageHandler);
}
