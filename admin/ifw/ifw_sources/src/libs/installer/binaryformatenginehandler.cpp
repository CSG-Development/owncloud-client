/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "binaryformatengine.h"
#include "binaryformatenginehandler.h"
#include "productkeycheck.h"

namespace QInstaller {

/*!
    \class QInstaller::BinaryFormatEngineHandler
    \inmodule QtInstallerFramework
    \brief The BinaryFormatEngineHandler class provides a way to register resource collections and
        resource files.
*/

/*!
    Creates a file engine for the file specified by \a fileName. To be able to create a file
    engine, the file name needs to be prefixed with \c {installer://}.

    Returns 0 if the engine cannot handle \a fileName.
*/
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
std::unique_ptr<QAbstractFileEngine> BinaryFormatEngineHandler::create(const QString &fileName) const
{
    return fileName.startsWith(QLatin1String("installer://"), Qt::CaseInsensitive )
        ? std::make_unique<BinaryFormatEngine>(m_resources, fileName) : nullptr;
}
#else
QAbstractFileEngine *BinaryFormatEngineHandler::create(const QString &fileName) const
{
    return fileName.startsWith(QLatin1String("installer://"), Qt::CaseInsensitive )
        ? new BinaryFormatEngine(m_resources, fileName) : nullptr;
}
#endif

/*!
    Clears the contents of the binary format engine.
*/
void BinaryFormatEngineHandler::clear()
{
    m_resources.clear();
}

/*!
    Returns the active instance of the engine.
*/
BinaryFormatEngineHandler *BinaryFormatEngineHandler::instance()
{
    static BinaryFormatEngineHandler instance;
    return &instance;
}

/*!
    Registers the given resource collections \a collections in the engine.
*/
void BinaryFormatEngineHandler::registerResources(const QList<ResourceCollection> &collections)
{
    foreach (const ResourceCollection &collection, collections) {
        if (ProductKeyCheck::instance()->isValidPackage(QString::fromUtf8(collection.name())))
            m_resources.insert(collection.name(), collection);
    }
}

/*!
    Registers the resource specified by \a resourcePath in a resource collection specified
    by \a fileName. The file name \a fileName must be in the form of \c {installer://}, followed
    by the collection name and resource name separated by a forward slash.

    A valid file name looks like this: installer://collectionName/resourceName
*/
void
BinaryFormatEngineHandler::registerResource(const QString &fileName, const QString &resourcePath)
{
    static const QChar sep = QChar::fromLatin1('/');
    static const QString prefix = QString::fromLatin1("installer://");
    Q_ASSERT(QStringView(fileName).startsWith(prefix, Qt::CaseInsensitive));

    // cut the prefix
    QString path = fileName.mid(prefix.length());
    while (path.endsWith(sep))
        path.chop(1);

    const QByteArray resourceName = path.section(sep, 1, 1).toUtf8();
    const QByteArray collectionName = path.section(sep, 0, 0).toUtf8();

    if (!ProductKeyCheck::instance()->isValidPackage(QString::fromUtf8(collectionName)))
        return;

    m_resources[collectionName].setName(collectionName);
    m_resources[collectionName].appendResource(QSharedPointer<Resource>(new Resource(resourcePath,
        resourceName)));
}

} // namespace QInstaller
