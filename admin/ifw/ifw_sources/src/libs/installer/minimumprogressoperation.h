/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef MINIMUMPROGRESSOPERATION_H
#define MINIMUMPROGRESSOPERATION_H

#include "qinstallerglobal.h"

#include <QtCore/QObject>

namespace QInstaller {

class MinimumProgressOperation : public QObject, public Operation
{
    Q_OBJECT

public:
    explicit MinimumProgressOperation(PackageManagerCore *core);

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;

signals:
    void progressChanged(double progress);
};

} // namespace QInstaller

#endif // MINIMUMPROGRESSOPERATION_H
