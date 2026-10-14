/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef DOWNLOADFILETASK_P_H
#define DOWNLOADFILETASK_P_H

#include "downloadfiletask.h"
#include <observer.h>

#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkInformation>
#include <QTimer>

#include <memory>
#include <unordered_map>

QT_BEGIN_NAMESPACE
class QSslError;
QT_END_NAMESPACE

namespace QInstaller {

struct Data
{
    Q_DISABLE_COPY(Data)

    Data()
        : file(Q_NULLPTR)
        , observer(Q_NULLPTR)
        , rangeRequestSent(false)
    {}

    Data(const FileTaskItem &fti)
        : taskItem(fti)
        , file(Q_NULLPTR)
        , observer(new FileTaskObserver(QCryptographicHash::Sha1))
        , rangeRequestSent(false)
    {}

    FileTaskItem taskItem;
    std::unique_ptr<QFile> file;
    std::unique_ptr<FileTaskObserver> observer;
    bool rangeRequestSent;
};

class Downloader : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Downloader)

public:
    Downloader();
    ~Downloader();

    void download(QFutureInterface<FileTaskResult> &fi, const QList<FileTaskItem> &items,
        QNetworkProxyFactory *networkProxyFactory, const bool progressValueInBytes);

signals:
    void finished();
    void progressChanged(const quint64 progress);
    void fileDownloaded(const QString &fileName, const QString &componentName);
    void networkDisconnected();

private slots:
    void doDownload();
    void onReadyRead();
    void onFinished();
    void errorOccurred(QNetworkReply::NetworkError error);
    void onSslErrors(const QList<QSslError> &sslErrors);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);
    void onProxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator);
    void onTimeout();
    void onReachabilityChanged(QNetworkInformation::Reachability newReachability);

protected:
    void timerEvent(QTimerEvent *event);

private:
    bool testCanceled();
    void shutDown();
    QNetworkReply *startDownload(const FileTaskItem &item);
    void connectNetworkReply(QNetworkReply *reply);
    void disconnectNetworkReply(QNetworkReply *reply);
    void resumeDownload();

    void runDownloadDeadlineTimer();
    void stopDownloadDeadlineTimer();
    void setDownloadPaused(bool paused);
    bool isDownloadPaused();
    void setDownloadResumed(bool resumed);
    bool isDownloadResumed();
    void updateBytesDownloadedBeforeResume(qint64 bytes);
    void updateTotalBytesDownloadedBeforeResume();

private:
    QFutureInterface<FileTaskResult> *m_futureInterface;

    QTimer m_timer;
    int m_finished;
    QNetworkAccessManager m_nam;
    QList<FileTaskItem> m_items;
    QMultiHash<QNetworkReply*, QUrl> m_redirects;
    std::unordered_map<QNetworkReply*, std::unique_ptr<Data>> m_downloads;
    qint64 m_progress;
    QBasicTimer m_downloadDeadlineTimer;
    int m_downloadDeadlineTimerInterval;
    bool m_downloadPaused;
    bool m_downloadResumed;
    bool m_progressValueInBytes;
};

}   // namespace QInstaller

#endif // DOWNLOADFILETASK_P_H
