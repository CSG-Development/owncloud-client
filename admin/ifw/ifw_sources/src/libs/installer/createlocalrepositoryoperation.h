/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef CREATELOCALREPOSITORYOPERATION_H
#define CREATELOCALREPOSITORYOPERATION_H

#include "qinstallerglobal.h"

#include <QtCore/QObject>

namespace QInstaller {

class INSTALLER_EXPORT CreateLocalRepositoryOperation : public QObject, public Operation
{
    Q_OBJECT
    friend struct AutoHelper;

public:
    explicit CreateLocalRepositoryOperation(PackageManagerCore *core);

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;

signals:
    void progressChanged(double progress);
    void outputTextChanged(const QString &message);

private:
    void emitFullProgress();
};

} // namespace QInstaller

#endif // CREATELOCALREPOSITORYOPERATION_H
