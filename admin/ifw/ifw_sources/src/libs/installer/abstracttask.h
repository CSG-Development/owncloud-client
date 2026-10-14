/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef ABSTRACTTASK_H
#define ABSTRACTTASK_H

#include "runextensions.h"

#include <QException>
#include <QFutureInterface>
#include <QVariant>

namespace QInstaller {

class AbstractTaskData
{
public:
    AbstractTaskData() {}
    virtual ~AbstractTaskData() = 0;

    QVariant value(int role) const { return m_data.value(role); }
    void insert(int key, const QVariant &value) { m_data.insert(key, value); }

private:
    QHash<int, QVariant> m_data;
};
inline AbstractTaskData::~AbstractTaskData() {}

class TaskException : public QException
{
public:
    TaskException() {}
    ~TaskException() throw() {}
    explicit TaskException(const QString &message)
        : m_message(message)
    {}

    void raise() const override { throw *this; }
    QString message() const { return m_message; }
    TaskException *clone() const override { return new TaskException(*this); }

private:
    QString m_message;
};

template <typename T>
class AbstractTask : public QObject
{
    Q_DISABLE_COPY(AbstractTask)

public:
    AbstractTask() {}
    virtual ~AbstractTask() {}

    virtual void doTask(QFutureInterface<T> &futureInterface) = 0;
};

}   // namespace QInstaller

#endif // ABSTRACTTASK_H
