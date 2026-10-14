/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef FILEDOWNLOADERFACTORY_H
#define FILEDOWNLOADERFACTORY_H

#include "genericfactory.h"
#include "updater.h"

#include <QtCore/QStringList>
#include <QtCore/QUrl>

#include <QtNetwork/QNetworkProxyFactory>

QT_BEGIN_NAMESPACE
class QObject;
QT_END_NAMESPACE

namespace KDUpdater {

class FileDownloader;

class KDTOOLS_EXPORT FileDownloaderProxyFactory : public QNetworkProxyFactory
{
public:
    virtual ~FileDownloaderProxyFactory() {}
    virtual FileDownloaderProxyFactory *clone() const = 0;
};

class KDTOOLS_EXPORT FileDownloaderFactory : public GenericFactory<FileDownloader, QString,
                                                                     QObject*>
{
    Q_DISABLE_COPY(FileDownloaderFactory)
    struct FileDownloaderFactoryData {
        FileDownloaderFactoryData() : m_factory(0) {}
        ~FileDownloaderFactoryData() { delete m_factory; }

        bool m_ignoreSslErrors;
        QStringList m_supportedSchemes;
        FileDownloaderProxyFactory *m_factory;
    };

public:
    static FileDownloaderFactory &instance();
    ~FileDownloaderFactory();

    template<typename T>
    void registerFileDownloader(const QString &scheme)
    {
        registerProduct<T>(scheme);
        d->m_supportedSchemes.append(scheme);
    }
    FileDownloader *create(const QString &scheme, QObject *parent = 0) const;

    static void setProxyFactory(FileDownloaderProxyFactory *factory);

    static QStringList supportedSchemes();
    static bool isSupportedScheme(const QString &scheme);

private:
    FileDownloaderFactory();

private:
    FileDownloaderFactoryData *d;
};

} // namespace KDUpdater

#endif // FILEDOWNLOADERFACTORY_H
