/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "linereplaceoperation.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

using namespace QInstaller;

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::LineReplaceOperation
    \internal
*/

LineReplaceOperation::LineReplaceOperation(PackageManagerCore *core)
    : UpdateOperation(core)
{
    setName(QLatin1String("LineReplace"));
}

void LineReplaceOperation::backup()
{
}

bool LineReplaceOperation::performOperation()
{
    // Arguments:
    // 1. filename
    // 2. startsWith Search-String
    // 3. Replace-Line-String
    if (!checkArgumentCount(3))
        return false;

    const QStringList args = arguments();
    const QString fileName = args.at(0);
    const QString searchString = args.at(1);
    const QString replaceString = args.at(2);

    if (searchString.isEmpty()) {
        setError(InvalidArguments);
        setErrorString(tr("Invalid argument in %1: Empty search "
            "argument is not supported.").arg(name()));

        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setError(UserDefinedError);
        setErrorString(tr("Cannot open file \"%1\" for reading: %2").arg(
                           QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    QString replacement;
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        if (line.trimmed().startsWith(searchString))
            replacement.append(replaceString + QLatin1String("\n"));
        else
            replacement.append(line + QLatin1String("\n"));
    }
    file.close();

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        setError(UserDefinedError);
        setErrorString(tr("Cannot open file \"%1\" for writing: %2").arg(
                           QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    stream.setDevice(&file);
    stream << replacement;
    file.close();

    return true;
}

bool LineReplaceOperation::undoOperation()
{
    return true;
}

bool LineReplaceOperation::testOperation()
{
    return true;
}
