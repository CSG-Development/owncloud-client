/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef CONCURRENTOPERATIONRUNNER_H
#define CONCURRENTOPERATIONRUNNER_H

#include "qinstallerglobal.h"

#include <QObject>
#include <QHash>
#include <QFutureWatcher>

namespace QInstaller {

class INSTALLER_EXPORT ConcurrentOperationRunner : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ConcurrentOperationRunner)

public:
    explicit ConcurrentOperationRunner(QObject *parent = nullptr);
    explicit ConcurrentOperationRunner(OperationList *operations,
        const Operation::OperationType type, QObject *parent = nullptr);
    ~ConcurrentOperationRunner();

    void setOperations(OperationList *operations);
    void setType(const Operation::OperationType type);
    void setMaxThreadCount(int count);

    QHash<Operation *, bool> run();

signals:
    void operationStarted(QInstaller::Operation *operation);
    void progressChanged(const int completed, const int total);
    void finished();

public slots:
    void cancel();

private slots:
    void onOperationStarted(QInstaller::Operation *operation);
    void onOperationfinished();

private:
    bool runOperation(Operation *const operation);
    void reset();

private:
    int m_completedOperations;
    int m_totalOperations;

    QHash<Operation *, QFutureWatcher<bool> *> m_operationWatchers;
    QHash<Operation *, bool> m_results;

    OperationList *m_operations;
    Operation::OperationType m_type;

    QThreadPool *const m_threadPool;
};

} // namespace QInstaller

#endif // CONCURRENTOPERATIONRUNNER_H
