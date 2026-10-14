/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "operationtracer.h"

#include "packagemanagercore.h"

namespace QInstaller {

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::AbstractOperationTracer
    \brief The AbstractOperationTracer is a pure virtual base class for classes
           tracing starting and finishing of installer operations.
*/

/*!
    \fn QInstaller::AbstractOperationTracer::~AbstractOperationTracer()

    Destructs object. A subclass may override this method.
*/

/*!
    \fn QInstaller::AbstractOperationTracer::trace(const QString &state)

    Prints trace output for starting operation in \a state.
    A subclass should implement this method.
*/

/*!
    Constructs tracer for \a operation. Objects of this class cannot
    be constructed directly, but the derived classes should explicitly call
    the base class constructor in their constructors.
*/
AbstractOperationTracer::AbstractOperationTracer(Operation *operation)
    : m_operation(nullptr)
{
    // don't create output for that hacky pseudo operation
    if (operation->name() != QLatin1String("MinimumProgress"))
        m_operation = operation;
}


/*!
    \inmodule QtInstallerFramework
    \class QInstaller::OperationTracer
    \brief The OperationTracer prints trace output for starting of operations
           and automatically indicates finish on destruction.
*/

/*!
    Constructs tracer for \a operation.
*/
OperationTracer::OperationTracer(Operation *operation)
    : AbstractOperationTracer(operation)
{
}

/*!
    Destructs object and prints message indicating finished operation.
*/
OperationTracer::~OperationTracer()
{
    if (!m_operation)
        return;

    qCDebug(QInstaller::lcInstallerInstallLog) << "Done";
}

/*!
    Prints trace output for starting operation in \a state.
*/
void OperationTracer::trace(const QString &state)
{
    if (!m_operation)
        return;

    qCDebug(lcInstallerInstallLog).noquote() << QString::fromLatin1("%1 %2 operation: %3")
        .arg(state, m_operation->value(QLatin1String("component")).toString(), m_operation->name());

    QStringList args = m_operation->arguments();
    if (m_operation->requiresUnreplacedVariables())
        args = m_operation->packageManager()->replaceVariables(m_operation->arguments());

    qCDebug(lcInstallerInstallLog).noquote() << QString::fromLatin1("\t- arguments: %1")
        .arg(args.join(QLatin1String(", ")));
}


/*!
    \inmodule QtInstallerFramework
    \class QInstaller::ConcurrentOperationTracer
    \brief The ConcurrentOperationTracer prints trace output for starting of
           asynchronous operations.
*/

/*!
    Constructs tracer for \a operation.
*/
ConcurrentOperationTracer::ConcurrentOperationTracer(Operation *operation)
    : AbstractOperationTracer(operation)
{
}

/*!
    Prints trace output for starting operation in \a state.
*/
void ConcurrentOperationTracer::trace(const QString &state)
{
    if (!m_operation)
        return;

    qCDebug(lcInstallerInstallLog).noquote() << QString::fromLatin1("%1 %2 concurrent operation: %3")
        .arg(state, m_operation->value(QLatin1String("component")).toString(), m_operation->name());

    QStringList args = m_operation->arguments();
    if (m_operation->requiresUnreplacedVariables())
        args = m_operation->packageManager()->replaceVariables(m_operation->arguments());

    qCDebug(lcInstallerInstallLog).noquote() << QString::fromLatin1("\t- arguments: %1")
        .arg(args.join(QLatin1String(", ")));

    qCDebug(QInstaller::lcInstallerInstallLog) << "Started";
}

} // namespace QInstaller
