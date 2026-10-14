/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef REMOTESERVERCONNECTION_P_H
#define REMOTESERVERCONNECTION_P_H

#include "protocol.h"
#include "libarchivearchive.h"

#include <QMutex>
#include <QProcess>
#include <QVariant>

namespace QInstaller {

class QProcessSignalReceiver : public QObject
{
    Q_OBJECT
    friend class RemoteServerConnection;

private:
    explicit QProcessSignalReceiver(QProcess *process)
        : QObject(process)
    {
        connect(process, &QIODevice::bytesWritten, this, &QProcessSignalReceiver::onBytesWritten);
        connect(process, &QIODevice::aboutToClose, this, &QProcessSignalReceiver::onAboutToClose);
        connect(process, &QIODevice::readChannelFinished, this, &QProcessSignalReceiver::onReadChannelFinished);
        connect(process, SIGNAL(errorOccurred(QProcess::ProcessError)),
            SLOT(onError(QProcess::ProcessError)));
        connect(process, &QProcess::readyReadStandardOutput,
                this, &QProcessSignalReceiver::onReadyReadStandardOutput);
        connect(process, &QProcess::readyReadStandardError,
                this, &QProcessSignalReceiver::onReadyReadStandardError);
        connect(process, SIGNAL(finished(int,QProcess::ExitStatus)),
                SLOT(onFinished(int,QProcess::ExitStatus)));
        connect(process, &QIODevice::readyRead, this, &QProcessSignalReceiver::onReadyRead);
        connect(process, &QProcess::started, this, &QProcessSignalReceiver::onStarted);
        connect(process, &QProcess::stateChanged,
            this, &QProcessSignalReceiver::onStateChanged);
    }

private Q_SLOTS:
    void onBytesWritten(qint64 count) {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalBytesWritten));
        m_receivedSignals.append(count);
    }

    void onAboutToClose() {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalAboutToClose));
    }

    void onReadChannelFinished() {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalReadChannelFinished));
    }

    void onError(QProcess::ProcessError error) {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalError));
        m_receivedSignals.append(static_cast<int> (error));
    }

    void onReadyReadStandardOutput() {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalReadyReadStandardOutput));
    }

    void onReadyReadStandardError() {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalReadyReadStandardError));
    }

    void onFinished(int exitCode, QProcess::ExitStatus exitStatus) {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalFinished));
        m_receivedSignals.append(exitCode);
        m_receivedSignals.append(static_cast<int> (exitStatus));
    }

    void onReadyRead() {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalReadyRead));
    }

    void onStarted() {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalStarted));
    }

    void onStateChanged(QProcess::ProcessState newState) {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::QProcessSignalStateChanged));
        m_receivedSignals.append(static_cast<int>(newState));
    }

private:
    QMutex m_lock;
    QVariantList m_receivedSignals;
};

class AbstractArchiveSignalReceiver : public QObject
{
    Q_OBJECT
    friend class RemoteServerConnection;

private:
    explicit AbstractArchiveSignalReceiver(LibArchiveArchive *archive)
        : QObject(archive)
    {
        connect(archive, &LibArchiveArchive::currentEntryChanged,
                this, &AbstractArchiveSignalReceiver::onCurrentEntryChanged);
        connect(archive, &LibArchiveArchive::completedChanged,
                this, &AbstractArchiveSignalReceiver::onCompletedChanged);
        connect(archive, &LibArchiveArchive::dataBlockRequested,
                this, &AbstractArchiveSignalReceiver::onDataBlockRequested);
        connect(archive, &LibArchiveArchive::seekRequested,
                this, &AbstractArchiveSignalReceiver::onSeekRequested);
        connect(archive, &LibArchiveArchive::workerFinished,
                this, &AbstractArchiveSignalReceiver::onWorkerFinished);
    }

private Q_SLOTS:
    void onCurrentEntryChanged(const QString &filename)
    {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::AbstractArchiveSignalCurrentEntryChanged));
        m_receivedSignals.append(filename);
    }

    void onCompletedChanged(quint64 completed, quint64 total)
    {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::AbstractArchiveSignalCompletedChanged));
        m_receivedSignals.append(completed);
        m_receivedSignals.append(total);
    }

    void onDataBlockRequested()
    {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::AbstractArchiveSignalDataBlockRequested));
    }

    void onSeekRequested(qint64 offset, int whence)
    {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::AbstractArchiveSignalSeekRequested));
        m_receivedSignals.append(offset);
        m_receivedSignals.append(whence);
    }

    void onWorkerFinished()
    {
        QMutexLocker _(&m_lock);
        m_receivedSignals.append(QLatin1String(Protocol::AbstractArchiveSignalWorkerFinished));
    }

private:
    QMutex m_lock;
    QVariantList m_receivedSignals;
};

} // namespace QInstaller

#endif // REMOTESERVERCONNECTION_P_H
