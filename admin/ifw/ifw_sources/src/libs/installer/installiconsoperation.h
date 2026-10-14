/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef INSTALLICONSOPERATION_H
#define INSTALLICONSOPERATION_H

#include "qinstallerglobal.h"

#include <QtCore/QObject>

namespace QInstaller {

class INSTALLER_EXPORT InstallIconsOperation : public QObject, public Operation
{
    Q_OBJECT
public:
    explicit InstallIconsOperation(PackageManagerCore *core);

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;

Q_SIGNALS:
    void outputTextChanged(const QString &progress);

private:
    QString targetDirectory();
};

}

#endif
