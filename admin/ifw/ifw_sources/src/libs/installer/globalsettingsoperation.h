/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef GLOBALSETTINGSOPERATION_H
#define GLOBALSETTINGSOPERATION_H

#include "qinstallerglobal.h"

namespace QInstaller {

class QSettingsWrapper;
class INSTALLER_EXPORT GlobalSettingsOperation : public Operation
{
    Q_DECLARE_TR_FUNCTIONS(QInstaller::GlobalSettingsOperation)
public:
    explicit GlobalSettingsOperation(PackageManagerCore *core);

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;

private:
    QSettingsWrapper *setup(QString *key, QString *value, const QStringList &args);
};

} // namespace QInstaller

#endif // GLOBALSETTINGSOPERATION_H
