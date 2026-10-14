/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef CONSUMEOUTPUTOPERATION_H
#define CONSUMEOUTPUTOPERATION_H

#include "qinstallerglobal.h"

namespace QInstaller {

class INSTALLER_EXPORT ConsumeOutputOperation : public Operation
{
    Q_DECLARE_TR_FUNCTIONS(QInstaller::ConsumeOutputOperation)
public:
    explicit ConsumeOutputOperation(PackageManagerCore *core);

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;

private:
};

} // namespace QInstaller

#endif // CONSUMEOUTPUTOPERATION_H
