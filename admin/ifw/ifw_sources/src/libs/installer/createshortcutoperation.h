/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef CREATESHORTCUTOPERATION_H
#define CREATESHORTCUTOPERATION_H

#include "qinstallerglobal.h"

namespace QInstaller {

class INSTALLER_EXPORT CreateShortcutOperation : public Operation
{
    Q_DECLARE_TR_FUNCTIONS(QInstaller::CreateShortcutOperation)
public:
    explicit CreateShortcutOperation(PackageManagerCore *core = nullptr);

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;

private:
    void ensureOptionalArgumentsRead();

    bool m_optionalArgumentsRead;
    QString m_iconId;
    QString m_iconPath;
    QString m_workingDir;
    QString m_description;
};

}

#endif
