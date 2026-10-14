/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#include "createlinkoperation.h"

#include "link.h"

#include <QDir>
#include <QFileInfo>

using namespace QInstaller;

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::CreateLinkOperation
    \internal
*/

CreateLinkOperation::CreateLinkOperation(PackageManagerCore *core)
    : UpdateOperation(core)
{
    setName(QLatin1String("CreateLink"));
}

void CreateLinkOperation::backup()
{
}

bool CreateLinkOperation::performOperation()
{
    if (!checkArgumentCount(2))
        return false;

    const QStringList args = arguments();
    const QString& linkPath = args.at(0);
    const QString& targetPath = args.at(1);
    Link link = Link::create(linkPath, targetPath);

    if (!link.exists()) {
        setError(UserDefinedError);
        setErrorString(tr("Cannot create link from \"%1\" to \"%2\".").arg(
                           QDir::toNativeSeparators(linkPath), QDir::toNativeSeparators(targetPath)));
        return false;
    }

    return true;
}

bool CreateLinkOperation::undoOperation()
{
    QStringList args = arguments();

    const QString& linkPath = args.at(0);
    const QString& targetPath = args.at(1);

    Link link = Link(linkPath);
    if (!link.exists()) {
        return true;
    }
    if (!link.remove()) {
        setError(UserDefinedError);
        setErrorString(tr("Cannot remove link from \"%1\" to \"%2\".").arg(
                           QDir::toNativeSeparators(linkPath), QDir::toNativeSeparators(targetPath)));
        return false;
    }

    return !QFileInfo::exists(linkPath);
}

bool CreateLinkOperation::testOperation()
{
    return true;
}
