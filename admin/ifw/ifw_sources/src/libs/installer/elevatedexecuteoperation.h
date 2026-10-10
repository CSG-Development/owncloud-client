/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef ELEVATEDEXECUTEOPERATION_H
#define ELEVATEDEXECUTEOPERATION_H

#include "qinstallerglobal.h"

namespace QInstaller {

class INSTALLER_EXPORT ElevatedExecuteOperation : public QObject, public Operation
{
    Q_OBJECT

public:
    enum Error {
        NoError = 0,
        Error,
        NeedsRerun
    };

    explicit ElevatedExecuteOperation(PackageManagerCore *core);
    ~ElevatedExecuteOperation();

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;

Q_SIGNALS:
    void cancelProcess();
    void outputTextChanged(const QString &text);

public Q_SLOTS:
    void cancelOperation();

private:
    Q_PRIVATE_SLOT(d, void readProcessOutput())

    class Private;
    Private *d;
};

} // namespace

#endif
