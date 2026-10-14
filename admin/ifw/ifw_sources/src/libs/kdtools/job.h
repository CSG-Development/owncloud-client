/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef JOB_H
#define JOB_H

#include "kdtoolsglobal.h"

#include <QtCore/QObject>

class KDTOOLS_EXPORT Job : public QObject
{
    Q_OBJECT
    class Private;

    Q_PROPERTY(int timeout READ timeout WRITE setTimeout)
    Q_PROPERTY(bool autoDelete READ autoDelete WRITE setAutoDelete)

public:
    explicit Job(QObject *parent = 0);
    ~Job();

    enum Error {
        NoError = 0,
        Canceled = 1,
        UserDefinedError = 128
    };

    enum Capability {
        NoCapabilities = 0x0,
        Cancelable = 0x1
    };

    Q_DECLARE_FLAGS(Capabilities, Capability)

    int error() const;
    QString errorString() const;

    bool autoDelete() const;
    void setAutoDelete(bool autoDelete);

    Capabilities capabilities() const;
    bool hasCapability(Capability c) const;

    void waitForStarted();
    void waitForFinished();

    quint64 totalAmount() const;
    quint64 processedAmount() const;

    int timeout() const;
    void setTimeout(int milliseconds);

public Q_SLOTS:
    void start();
    void cancel();

Q_SIGNALS:
    void started(Job *job);
    void finished(Job *job);

    void infoMessage(Job *job, const QString &message);
    void progress(Job *job, quint64 processed, quint64 total);
    void totalProgress(quint64 total);

protected:
    virtual void doStart() = 0;
    virtual void doCancel() = 0;

    void setCapabilities(Capabilities c);

    void setTotalAmount(quint64 amount);
    void setProcessedAmount(quint64 amount);

    void setError(int error);
    void setErrorString(const QString &errorString);

    void emitFinished();
    void emitFinishedWithError(int error, const QString &errorString);

private Q_SLOTS:
    void onFinished();

private:
    Private *d;
    Q_PRIVATE_SLOT(d, void delayedStart())
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Job::Capabilities)

#endif // JOB_H
