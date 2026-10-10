/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef OPERATIONRUNNER_H
#define OPERATIONRUNNER_H

#include <binaryformat.h>

#include <QObject>

namespace QInstaller {
    struct OperationBlob;
    class PackageManagerCore;
}

class OperationRunner : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OperationRunner)

public:
    enum struct RunMode {
        Do,
        Undo
    };

    OperationRunner(qint64 magicMarker, const QList<QInstaller::OperationBlob> &oldOperations);
    ~OperationRunner();
    int runOperation(QStringList arguments, RunMode mode);

private slots:
    void print(const QString &value);

private:
    QInstaller::PackageManagerCore *m_core;
};

#endif // OPERATIONRUNNER_H
