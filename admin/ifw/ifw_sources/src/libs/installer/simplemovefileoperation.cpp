/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "simplemovefileoperation.h"

#include <QDir>
#include <QtCore/QFileInfo>

namespace QInstaller {

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::SimpleMoveFileOperation
    \internal
*/

SimpleMoveFileOperation::SimpleMoveFileOperation(PackageManagerCore *core)
    : UpdateOperation(core)
{
    setName(QLatin1String("SimpleMoveFile"));
}

void SimpleMoveFileOperation::backup()
{
}

bool SimpleMoveFileOperation::performOperation()
{
    if (!checkArgumentCount(2))
        return false;

    const QStringList args = arguments();
    const QString source = args.at(0);
    const QString target = args.at(1);

    if (source.isEmpty() || target.isEmpty()) {
        setError(UserDefinedError);
        setErrorString(tr("None of the arguments can be empty: source \"%1\", target \"%2\".")
            .arg(QDir::toNativeSeparators(source), QDir::toNativeSeparators(target)));
        return false;
    }

    // If destination file exists, then we cannot use QFile::copy() because it does not overwrite an existing
    // file. So we remove the destination file.
    QFile file(target);
    if (file.exists()) {
        if (!file.remove()) {
            setError(UserDefinedError);
            setErrorString(tr("Cannot move file from \"%1\" to \"%2\", because the target path exists and is "
                "not removable.").arg(QDir::toNativeSeparators(source), QDir::toNativeSeparators(target)));
            return false;
        }
    }

    file.setFileName(source);
    if (!file.rename(target)) {
        setError(UserDefinedError);
        setErrorString(tr("Cannot move file \"%1\" to \"%2\": %3").arg(
                           QDir::toNativeSeparators(source), QDir::toNativeSeparators(target),
                           file.errorString()));
        return false;
    }

    emit outputTextChanged(tr("Moving file \"%1\" to \"%2\".").arg(QDir::toNativeSeparators(source),
                                                                   QDir::toNativeSeparators(target)));
    return true;
}

bool SimpleMoveFileOperation::undoOperation()
{
    if (skipUndoOperation())
        return true;
    const QString source = arguments().at(0);
    const QString target = arguments().at(1);

    QFile(target).rename(source);
    emit outputTextChanged(tr("Moving file \"%1\" to \"%2\".").arg(QDir::toNativeSeparators(target),
                                                                   QDir::toNativeSeparators(source)));

    return true;
}

bool SimpleMoveFileOperation::testOperation()
{
    return true;
}

}   // namespace QInstaller
