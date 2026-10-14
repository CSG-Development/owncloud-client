/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef OPERATIONTRACER_H
#define OPERATIONTRACER_H

#include "qinstallerglobal.h"
#include "globals.h"

namespace QInstaller {

class INSTALLER_EXPORT AbstractOperationTracer
{
public:
    explicit AbstractOperationTracer(Operation *operation);
    virtual ~AbstractOperationTracer() = default;

    virtual void trace(const QString &state) = 0;

protected:
    Operation *m_operation;
};

class INSTALLER_EXPORT OperationTracer : public AbstractOperationTracer
{
public:
    explicit OperationTracer(Operation *operation);
    ~OperationTracer() override;

    void trace(const QString &state) override;
};

class INSTALLER_EXPORT ConcurrentOperationTracer : public AbstractOperationTracer
{
public:
    explicit ConcurrentOperationTracer(Operation *operation);

    void trace(const QString &state) override;
};

} // namespace QInstaller

#endif // OPERATIONTRACER_H
