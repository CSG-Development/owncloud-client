/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef REGISTERFILETYPEOPERATION_H
#define REGISTERFILETYPEOPERATION_H

#include "qinstallerglobal.h"

namespace QInstaller {

class INSTALLER_EXPORT RegisterFileTypeOperation : public QObject, public Operation
{
    Q_OBJECT

public:
    explicit RegisterFileTypeOperation(PackageManagerCore *core);

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;

private:
    void ensureOptionalArgumentsRead();

    bool m_optionalArgumentsRead;
    QString m_progId;
};

}

#endif
