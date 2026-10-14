/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef OBSERVER_H
#define OBSERVER_H

#include <QCryptographicHash>
#include <QObject>

namespace QInstaller {

class Observer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Observer)

public:
    Observer() {}
    virtual ~Observer() {}

    virtual int progressValue() const = 0;
    virtual QString progressText() const = 0;
};

class FileTaskObserver : public Observer
{
    Q_OBJECT
    Q_DISABLE_COPY(FileTaskObserver)

public:
    FileTaskObserver(QCryptographicHash::Algorithm algorithm);
    ~FileTaskObserver();

    int progressValue() const override;
    QString progressText() const override;

    QByteArray checkSum() const;
    void addCheckSumData(const QByteArray &data);
    void resetCheckSumData();

    QByteArray expectedSha1() const;
    void setExpectedSha1(const QByteArray &data);

    void addSample(qint64 sample);
    void timerEvent(QTimerEvent *event) override;

    void setBytesTransfered(qint64 bytesTransfered);
    void addBytesTransfered(qint64 bytesTransfered);
    void setBytesToTransfer(qint64 bytesToTransfer);

    qint64 bytesTransfered() const;
    qint64 bytesToTransfer() const;

    void setDownloadFinished(bool downloadFinished);
    bool downloadFinished() const;

    qint64 totalBytesDownloadedBeforeResume() const;
    void updateBytesDownloadedBeforeResume(qint64 bytes);
    void updateTotalBytesDownloadedBeforeResume();

private:
    void init();

private:
    int m_timerId;
    int m_timerInterval;

    qint64 m_bytesTransfered;
    qint64 m_bytesToTransfer;
    qint64 m_bytesBeforeResume;
    qint64 m_totalBytesBeforeResume;

    qint64 m_samples[50];
    quint32 m_sampleIndex;
    qint64 m_bytesPerSecond;
    qint64 m_currentSpeedBin;

    QCryptographicHash m_hash;
    QByteArray m_expectedSha1;

    bool m_downloadFinished;
    bool m_informProgressInBytes;
};

}   // namespace QInstaller

#endif // OBSERVER_H
