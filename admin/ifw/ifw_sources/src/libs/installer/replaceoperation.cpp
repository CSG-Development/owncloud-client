/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "replaceoperation.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QRegularExpression>

using namespace QInstaller;

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::ReplaceOperation
    \internal
*/

ReplaceOperation::ReplaceOperation(PackageManagerCore *core)
    : UpdateOperation(core)
{
    setName(QLatin1String("Replace"));
}

void ReplaceOperation::backup()
{
}

bool ReplaceOperation::performOperation()
{
    static const QLatin1String stringMode("string");
    static const QLatin1String regexMode("regex");

    // Arguments:
    // 1. filename
    // 2. Source-String
    // 3. Replace-String
    // 4. mode=string|regex

    // Calling with three arguments defaults to string search,
    // this provides backward compatibility with the old syntax.
    if (!checkArgumentCount(3, 4))
        return false;

    const QStringList args = arguments();
    const QString fileName = args.at(0);
    const QString before = args.at(1);
    const QString after = args.at(2);
    QString mode = args.value(3);

    if (mode.isEmpty())
        mode = stringMode;

    if (before.isEmpty()) {
        setError(InvalidArguments);
        setErrorString(tr("Current search argument calling \"%1\" with "
            "empty search argument is not supported.").arg(name()));

        return false;
    }

    if (!(mode == stringMode || mode == regexMode)) {
        setError(InvalidArguments);
        setErrorString(tr("Current mode argument calling \"%1\" with "
            "arguments \"%2\" is not supported. Please use string or regex.")
            .arg(name(), arguments().join(QLatin1String("; "))));

        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(UserDefinedError);
        setErrorString(tr("Cannot open file \"%1\" for reading: %2").arg(
                           QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    QTextStream stream(&file);
    QString replacedFileContent = stream.readAll();
    file.close();

    if (!file.open(QIODevice::WriteOnly)) {
        setError(UserDefinedError);
        setErrorString(tr("Cannot open file \"%1\" for writing: %2").arg(
                           QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    stream.setDevice(&file);
    if (mode == regexMode) {
        QRegularExpression regex(before);
        stream << replacedFileContent.replace(regex, after);
    } else if (mode == stringMode) {
        stream << replacedFileContent.replace(before, after);
    }
    file.close();

    return true;
}

bool ReplaceOperation::undoOperation()
{
    // Need to remove settings again
    return true;
}

bool ReplaceOperation::testOperation()
{
    return true;
}
