/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef FILEDOWNLOADER_P_H
#define FILEDOWNLOADER_P_H

#include "filedownloader.h"

#include <QtNetwork/QNetworkReply>
#include <QNetworkAccessManager>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QNetworkInformation>
#endif

// these classes are not a part of the public API

namespace KDUpdater {

class LocalFileDownloader : public FileDownloader
{
    Q_OBJECT

public:
    explicit LocalFileDownloader(QObject *parent = 0);
    ~LocalFileDownloader();

public:
    LocalFileDownloader *clone(QObject *parent = 0) const override;
    void reset() override;

private Q_SLOTS:
    bool doDownload(DownloadType downloadtype) override;

private:
    struct Private;
    Private *d;
};

class HttpDownloader : public FileDownloader
{
    Q_OBJECT

public:
    explicit HttpDownloader(QObject *parent = 0);
    ~HttpDownloader();

public:
    HttpDownloader *clone(QObject *parent = 0) const override;
    void reset() override;

private Q_SLOTS:
    bool doDownload(DownloadType downloadtype) override;

private:
    void startDownload(const QUrl &url);

private:
    struct Private;
    Private *d;
};

} // namespace KDUpdater

#endif // FILEDOWNLOADER_P_H
