/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef FAKESTOPPROCESSFORUPDATEOPERATION_H
#define FAKESTOPPROCESSFORUPDATEOPERATION_H

#include "qinstallerglobal.h"

namespace QInstaller {

class INSTALLER_EXPORT FakeStopProcessForUpdateOperation : public QObject, public Operation
{
    Q_OBJECT

public:
    explicit FakeStopProcessForUpdateOperation(PackageManagerCore *core);

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;
};

}

#endif // FAKESTOPPROCESSFORUPDATEOPERATION_H
