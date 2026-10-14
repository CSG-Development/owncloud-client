/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "registerfiletypeoperation.h"

#include "constants.h"
#include "packagemanagercore.h"
#include "qsettingswrapper.h"

using namespace QInstaller;

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::RegisterFileTypeOperation
    \internal
*/

#ifdef Q_OS_WIN
#include <shlobj.h>


struct StartsWithProgId
{
    bool operator()(const QString &s)
    {
        return s.startsWith(QLatin1String("ProgId="));
    }
};

static QString takeProgIdArgument(QStringList &args)
{
    // if the arguments contain an option in the form "ProgId=...", find it and consume it
    QStringList::iterator it = std::find_if (args.begin(), args.end(), StartsWithProgId());
    if (it == args.end())
        return QString();

    const QString progId = it->mid(QString::fromLatin1("ProgId=").size());
    args.erase(it);
    return progId;
}

static QVariantHash readHive(QSettingsWrapper *const settings, const QString &hive)
{
    QVariantHash keyValues;

    settings->beginGroup(hive);
    foreach (const QString &key, settings->allKeys())
        keyValues.insert(key, settings->value(key));
    settings->endGroup();

    return keyValues;
}
#endif


// -- RegisterFileTypeOperation

RegisterFileTypeOperation::RegisterFileTypeOperation(PackageManagerCore *core)
    : UpdateOperation(core)
    , m_optionalArgumentsRead(false)
{
    setName(QLatin1String("RegisterFileType"));
}

void RegisterFileTypeOperation::backup()
{
}

bool RegisterFileTypeOperation::performOperation()
{
#ifdef Q_OS_WIN
    ensureOptionalArgumentsRead();
    if (!checkArgumentCount(2, 5, QString::fromLatin1("<extension> <command> [description [contentType [icon]]]")))
        return false;
    QStringList args = parsePerformOperationArguments();

    bool allUsers = false;
    PackageManagerCore *const core = packageManager();
    if (core && core->value(scAllUsers) == scTrue)
        allUsers = true;

    QSettingsWrapper settings(QLatin1String(allUsers ? "HKEY_LOCAL_MACHINE" : "HKEY_CURRENT_USER")
        , QSettings::NativeFormat);

    const QString classesProgId = QString::fromLatin1("Software/Classes/") + m_progId;
    const QString classesFileType = QString::fromLatin1("Software/Classes/.%2").arg(args.at(0));
    const QString classesApplications = QString::fromLatin1("Software/Classes/Applications/") + m_progId;

    // backup old value
    setValue(QLatin1String("oldType"), readHive(&settings, classesFileType));

    // register new values
    settings.setValue(QString::fromLatin1("%1/Default").arg(classesFileType), m_progId);
    settings.setValue(QString::fromLatin1("%1/OpenWithProgIds/%2").arg(classesFileType, m_progId), QString());
    settings.setValue(QString::fromLatin1("%1/shell/Open/Command/Default").arg(classesProgId), args.at(1));
    settings.setValue(QString::fromLatin1("%1/shell/Open/Command/Default").arg(classesApplications), args.at(1));

    // content type (optional)
    const QString contentType = args.value(3);
    if (!contentType.isEmpty())
        settings.setValue(QString::fromLatin1("%1/Content Type").arg(classesFileType), contentType);

    // description (optional)
    const QString description = args.value(2);
    if (!description.isEmpty())
        settings.setValue(QString::fromLatin1("%1/Default").arg(classesProgId), description);

     // icon (optional)
    const QString icon = args.value(4);
    if (!icon.isEmpty())
        settings.setValue(QString::fromLatin1("%1/DefaultIcon/Default").arg(classesProgId), icon);

    // backup new value
    setValue(QLatin1String("newType"), readHive(&settings, classesFileType));

    // force the shell to invalidate its cache
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);

    return true;
#else
    setError(UserDefinedError);
    setErrorString(tr("Registering file types is only supported on Windows."));
    return false;
#endif
}

bool RegisterFileTypeOperation::undoOperation()
{
#ifdef Q_OS_WIN
    ensureOptionalArgumentsRead();
    if (skipUndoOperation())
        return true;

    QStringList args = arguments();
    if (!checkArgumentCount(2, 5, tr("Register File Type: Invalid arguments")))
        return false;

    bool allUsers = false;
    PackageManagerCore *const core = packageManager();
    if (core && core->value(scAllUsers) == scTrue)
        allUsers = true;

    QSettingsWrapper settings(QLatin1String(allUsers ? "HKEY_LOCAL_MACHINE" : "HKEY_CURRENT_USER")
        , QSettings::NativeFormat);

    const QString classesProgId = QString::fromLatin1("Software/Classes/") + m_progId;
    const QString classesFileType = QString::fromLatin1("Software/Classes/.%2").arg(args.at(0));
    const QString classesApplications = QString::fromLatin1("Software/Classes/Applications/") + m_progId;

    // Quoting MSDN here: When uninstalling an application, the ProgIDs and most other registry information
    // associated with that application should be deleted as part of the uninstallation.However, applications
    // that have taken ownership of a file type (by setting the Default value of the file type's
    // HKEY...\.extension subkey to the ProgID of the application) should not attempt to remove that value
    // when uninstalling. Leaving the data in place for the Default value avoids the difficulty of
    // determining whether another application has taken ownership of the file type and overwritten the
    // Default value after the original application was installed. Windows respects the Default value only
    // if the ProgID found there is a registered ProgID. If the ProgID is unregistered, it is ignored.

    // if the hive didn't change since we touched it
    if (value(QLatin1String("newType")).toHash() == readHive(&settings, classesFileType)) {
        // reset to the values we found
        settings.remove(classesFileType);
        settings.beginGroup(classesFileType);
        const QVariantHash keyValues = value(QLatin1String("oldType")).toHash();
        for (auto it = keyValues.cbegin(); it != keyValues.cend(); ++it) {
            const QString &key = it.key();
            settings.setValue(key, keyValues.value(key));
        }
        settings.endGroup();
    } else {
        // some changes happened, remove the only save value we know about
        settings.remove(QString::fromLatin1("%1/OpenWithProgIds/%2").arg(classesFileType, m_progId));
    }

    // remove ProgId and Applications entry
    settings.remove(classesProgId);
    settings.remove(classesApplications);

    // force the shell to invalidate its cache
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);

    return true;
#else
    setErrorString(tr("Registering file types is only supported on Windows."));
    return false;
#endif
}

bool RegisterFileTypeOperation::testOperation()
{
    return true;
}

void RegisterFileTypeOperation::ensureOptionalArgumentsRead()
{
#ifdef Q_OS_WIN
    if (m_optionalArgumentsRead)
        return;

    m_optionalArgumentsRead = true;

    QStringList args = arguments();

    m_progId = takeProgIdArgument(args);

    if (m_progId.isEmpty() && args.count() > 0)
        m_progId = QString::fromLatin1("%1_auto_file").arg(args.at(0));

    setArguments(args);
#endif
}
