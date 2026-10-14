/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef STOPPROCESSFORUPDATEOPERATION_H
#define STOPPROCESSFORUPDATEOPERATION_H

#include "qinstallerglobal.h"

namespace QInstaller {

class INSTALLER_EXPORT StopProcessForUpdateOperation : public Operation
{
public:
    explicit StopProcessForUpdateOperation(PackageManagerCore *core = nullptr);

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;
};

} // namespace QInstaller

#endif // STOPPROCESSFORUPDATEOPERATION_H
