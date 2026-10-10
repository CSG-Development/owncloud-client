/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "filedownloaderfactory.h"
#include "filedownloader_p.h"
#include "globals.h"

#include <QtNetwork/QSslSocket>

using namespace KDUpdater;

/*!
   \inmodule kdupdater
   \class KDUpdater::FileDownloaderFactory
   \brief The FileDownloaderFactory class acts as a factory for KDUpdater::FileDownloader.

   You can register one or more file downloaders with this factory and query them based on their
   scheme. The class follows the singleton design pattern. Only one instance of this class can
   be created and its reference can be fetched from the instance() method.
*/

/*!
    Returns the file downloader factory instance.
*/
FileDownloaderFactory& FileDownloaderFactory::instance()
{
    static KDUpdater::FileDownloaderFactory theFactory;
    return theFactory;
}

/*!
    Constructs a file downloader factory and registers the default file downloader set.
*/
FileDownloaderFactory::FileDownloaderFactory()
    : d (new FileDownloaderFactoryData)
{
    // Register the default file downloader set
    registerFileDownloader<LocalFileDownloader>( QLatin1String("file"));
    registerFileDownloader<HttpDownloader>(QLatin1String("ftp"));
    registerFileDownloader<HttpDownloader>(QLatin1String("http"));

#ifndef QT_NO_SSL
    if (QSslSocket::supportsSsl())
        registerFileDownloader<HttpDownloader>(QLatin1String("https"));
    else
        qCWarning(QInstaller::lcInstallerInstallLog) << "Cannot register file downloader for https protocol: QSslSocket::supportsSsl() returns false";
#endif

}

/*!
    Sets \a factory as the file downloader proxy factory.
*/
void FileDownloaderFactory::setProxyFactory(FileDownloaderProxyFactory *factory)
{
    delete FileDownloaderFactory::instance().d->m_factory;
    FileDownloaderFactory::instance().d->m_factory = factory;
}

/*!
    Destroys the file downloader factory.
*/
FileDownloaderFactory::~FileDownloaderFactory()
{
    delete d;
}

/*!
    Returns a list of supported schemes.
*/
QStringList FileDownloaderFactory::supportedSchemes()
{
    return FileDownloaderFactory::instance().d->m_supportedSchemes;
}

/*!
    Returns \c true if \a scheme is a supported scheme.
*/
bool FileDownloaderFactory::isSupportedScheme(const QString &scheme)
{
    return FileDownloaderFactory::instance().d->m_supportedSchemes.contains(scheme
        , Qt::CaseInsensitive);
}

/*!
     Returns a new instance of a KDUpdater::FileDownloader subclass. The
     instantiation of a subclass depends on the communication protocol string
     stored in \a scheme with the parent \a parent.

     \note Ownership of the created object remains with the programmer.
*/
FileDownloader *FileDownloaderFactory::create(const QString &scheme, QObject *parent) const
{
    FileDownloader *downloader = GenericFactory<FileDownloader, QString, QObject*>::create(scheme, parent);
    if (downloader && d->m_factory) {
        downloader->setScheme(scheme);
        downloader->setProxyFactory(d->m_factory->clone());
    }
    return downloader;
}

/*!
    \fn void KDUpdater::FileDownloaderFactory::registerFileDownloader(const QString &scheme)

    Registers a new file downloader with the factory based on \a scheme. If there is already
    a downloader with the same scheme, the downloader is replaced. When create() is called
    with that \a scheme, the file downloader is constructed using its default constructor.
*/

/*!
    \inmodule kdupdater
    \class KDUpdater::FileDownloaderProxyFactory
    \brief The FileDownloaderProxyFactory class provides fine-grained proxy selection.

    File downloader objects use a proxy factory to determine a more specific
    list of proxies to be used for a given request, instead of trying to use the
    same proxy value for all requests. This might only be of use for HTTP or FTP
    requests.
*/

/*!
    \fn KDUpdater::FileDownloaderProxyFactory::~FileDownloaderProxyFactory()

    Destroys the file downloader proxy factory.
*/

/*!
    \fn KDUpdater::FileDownloaderProxyFactory::clone() const

    Clones a file downloader proxy factory.
*/
