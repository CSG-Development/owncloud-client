/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef UPDATEOPERATIONS_H
#define UPDATEOPERATIONS_H

#include "updateoperation.h"

namespace KDUpdater {

class KDTOOLS_EXPORT CopyOperation : public UpdateOperation
{
    Q_DECLARE_TR_FUNCTIONS(KDUpdater::CopyOperation)
public:
    explicit CopyOperation(QInstaller::PackageManagerCore *core = 0);
    ~CopyOperation();

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;

    QDomDocument toXml() const override;
private:
    QString sourcePath();
    QString destinationPath();
};

class KDTOOLS_EXPORT MoveOperation : public UpdateOperation
{
    Q_DECLARE_TR_FUNCTIONS(KDUpdater::MoveOperation)
public:
    explicit MoveOperation(QInstaller::PackageManagerCore *core = 0);
    ~MoveOperation();

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;
};

class KDTOOLS_EXPORT DeleteOperation : public UpdateOperation
{
    Q_DECLARE_TR_FUNCTIONS(KDUpdater::DeleteOperation)
public:
    explicit DeleteOperation(QInstaller::PackageManagerCore *core = 0);
    ~DeleteOperation();

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;

    QDomDocument toXml() const override;
};

class KDTOOLS_EXPORT MkdirOperation : public UpdateOperation
{
    Q_DECLARE_TR_FUNCTIONS(KDUpdater::MkdirOperation)
public:
    explicit MkdirOperation(QInstaller::PackageManagerCore *core = 0);

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;
};

class KDTOOLS_EXPORT RmdirOperation : public UpdateOperation
{
    Q_DECLARE_TR_FUNCTIONS(KDUpdater::RmdirOperation)
public:
    RmdirOperation(QInstaller::PackageManagerCore *core = 0);

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;
};

class KDTOOLS_EXPORT AppendFileOperation : public UpdateOperation
{
    Q_DECLARE_TR_FUNCTIONS(KDUpdater::AppendFileOperation)
public:
    explicit AppendFileOperation(QInstaller::PackageManagerCore *core = 0);
    ~AppendFileOperation();

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;
};

class KDTOOLS_EXPORT PrependFileOperation : public UpdateOperation
{
    Q_DECLARE_TR_FUNCTIONS(KDUpdater::PrependFileOperation)
public:
    explicit PrependFileOperation(QInstaller::PackageManagerCore *core = 0);
    ~PrependFileOperation();

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;
};

} // namespace KDUpdater

#endif // UPDATEOPERATIONS_H
