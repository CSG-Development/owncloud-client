/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef QPROCESSWRAPPER_H
#define QPROCESSWRAPPER_H

#include "remoteobject.h"

#include <QIODevice>
#include <QProcess>
#include <QReadWriteLock>
#include <QTimer>

namespace QInstaller {

class INSTALLER_EXPORT QProcessWrapper : public RemoteObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QProcessWrapper)

public:
    enum ProcessState {
        NotRunning,
        Starting,
        Running
    };

    enum ExitStatus {
        NormalExit,
        CrashExit
    };

    enum ProcessChannel {
        StandardOutput = 0,
        StandardError = 1
    };

    enum ProcessChannelMode {
        SeparateChannels = 0,
        MergedChannels = 1,
        ForwardedChannels = 2
    };

    explicit QProcessWrapper(QObject *parent = 0);
    ~QProcessWrapper();

    int exitCode() const;
    ProcessState state() const;
    ExitStatus exitStatus() const;

    QString workingDirectory() const;
    void setWorkingDirectory(const QString &dir);

    QStringList environment() const;
    void setEnvironment(const QStringList &environment);

    QProcessWrapper::ProcessChannel readChannel() const;
    void setReadChannel(QProcessWrapper::ProcessChannel channel);

    QProcessWrapper::ProcessChannelMode processChannelMode() const;
    void setProcessChannelMode(QProcessWrapper::ProcessChannelMode channel);

    bool waitForStarted(int msecs = 30000);
    bool waitForFinished(int msecs = 30000);

    void start(const QString &program, const QStringList &arguments,
        QIODevice::OpenMode mode = QIODevice::ReadWrite);
    void start(const QString &program, QIODevice::OpenMode mode = QIODevice::ReadWrite);

    void closeWriteChannel();
    void kill();
    void terminate();
    QByteArray readAll();
    QByteArray readAllStandardOutput();
    QByteArray readAllStandardError();

    static bool startDetached(const QString &program);
    static bool startDetached(const QString &program, const QStringList &arguments);
    static bool startDetached(const QString &program, const QStringList &arguments,
        const QString &workingDirectory, qint64 *pid = 0);

    static bool startDetached2(const QString &program);
    static bool startDetached2(const QString &program, const QStringList &arguments);
    static bool startDetached2(const QString &program, const QStringList &arguments,
        const QString &workingDirectory, qint64 *pid = 0);

    QString errorString() const;
    qint64 write(const QByteArray &byteArray);
#ifdef Q_OS_WIN
    void setNativeArguments(const QString &arguments);
#endif

Q_SIGNALS:
    void bytesWritten(qint64);
    void aboutToClose();
    void readChannelFinished();
    void errorOccurred(QProcess::ProcessError);
    void readyReadStandardOutput();
    void readyReadStandardError();
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
    void readyRead();
    void started();
    void stateChanged(QProcess::ProcessState newState);

public Q_SLOTS:
    void cancel();

private slots:
    void processSignals();

private:
    QTimer m_timer;
    QProcess process;
    mutable QReadWriteLock m_lock;
};

} // namespace QInstaller

Q_DECLARE_METATYPE(QProcess::ExitStatus)
Q_DECLARE_METATYPE(QProcess::ProcessError)
Q_DECLARE_METATYPE(QProcess::ProcessState)

#endif  // QPROCESSWRAPPER_H
