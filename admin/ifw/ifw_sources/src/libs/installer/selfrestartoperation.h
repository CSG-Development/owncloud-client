/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef SELFRESTARTOPERATION_H
#define SELFRESTARTOPERATION_H

#include "qinstallerglobal.h"

namespace QInstaller {

class INSTALLER_EXPORT SelfRestartOperation : public Operation
{
    Q_DECLARE_TR_FUNCTIONS(QInstaller::SelfRestartOperation)
public:
    explicit SelfRestartOperation(PackageManagerCore *core);

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;
};

}

#endif
