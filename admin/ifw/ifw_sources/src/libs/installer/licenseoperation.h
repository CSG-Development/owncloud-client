/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef LICENSEOPERATION_H
#define LICENSEOPERATION_H

#include "qinstallerglobal.h"

namespace QInstaller {

class INSTALLER_EXPORT LicenseOperation : public Operation
{
    Q_DECLARE_TR_FUNCTIONS(QInstaller::LicenseOperation)
public:
    explicit LicenseOperation(PackageManagerCore *core);

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;
};

}   // namespace QInstaller

#endif //LICENSEOPERATION_H
