/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef QTCONCURRENT_RUNEX_H
#define QTCONCURRENT_RUNEX_H

#include <qrunnable.h>
#include <qfutureinterface.h>
#include <qthreadpool.h>

QT_BEGIN_NAMESPACE

namespace QtConcurrent {

template <typename T,  typename FunctionPointer, typename... Args>
class StoredInterfaceFunctionCall : public QRunnable
{
public:
    StoredInterfaceFunctionCall(void (fn)(QFutureInterface<T> &, Args...), const Args&&... args)
    : m_fn(fn), m_args(std::make_tuple(std::forward<Args>(args)...)) { }

    QFuture<T> start()
    {
        m_futureInterface.reportStarted();
        QFuture<T> future = m_futureInterface.future();
        QThreadPool::globalInstance()->start(this);
        return future;
    }

    void run() override
    {
        fn(m_futureInterface, std::forward<Args>(m_args)...);
        m_futureInterface.reportFinished();
    }
private:
    QFutureInterface<T> m_futureInterface;
    FunctionPointer m_fn;
    std::tuple<Args...> m_args;
};
template <typename T,  typename FunctionPointer, typename Class, typename... Args>
class StoredInterfaceMemberFunctionCall : public QRunnable
{
public:
    StoredInterfaceMemberFunctionCall(void (Class::*fn)(QFutureInterface<T> &, Args...), Class *object, const Args&&... args)
    : m_fn(fn), m_object(object), m_args(std::make_tuple(std::forward<Args>(args)...)) { }

    QFuture<T> start()
    {
        m_futureInterface.reportStarted();
        QFuture<T> future = m_futureInterface.future();
        QThreadPool::globalInstance()->start(this);
        return future;
    }

    void run() override
    {
        (m_object->*m_fn)(m_futureInterface, std::forward<Args>(m_args)...);
        m_futureInterface.reportFinished();
    }
private:
    QFutureInterface<T> m_futureInterface;
    FunctionPointer m_fn;
    Class *m_object;
    std::tuple<Args...> m_args;
};

template <typename T, typename... Args>
QFuture<T> run(void (*functionPointer)(QFutureInterface<T> &, Args...), Args... args)
{
    return (new StoredInterfaceFunctionCall<T, void (*)(QFutureInterface<T> &, Args...), Args...>(functionPointer, args...))->start();
}

template <typename Class, typename T, typename... Args>
QFuture<T> run(void (Class::*fn)(QFutureInterface<T> &, Args...), Class *object, Args... args)
{
    return (new StoredInterfaceMemberFunctionCall<T, void (Class::*)(QFutureInterface<T> &, Args...), Class, Args...>(fn, object, args...))->start();
}

} // namespace QtConcurrent

QT_END_NAMESPACE

#endif // QTCONCURRENT_RUNEX_H
