/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "binarydump.h"

#include <copydirectoryoperation.h>
#include <errors.h>
#include <fileio.h>
#include <binarycontent.h>

#include <QDirIterator>
#include <QDomDocument>
#include <QResource>

#include <iostream>

using namespace QInstallerTools;

int BinaryDump::dump(const QInstaller::ResourceCollectionManager &manager, const QString &target)
{
    QDir targetDir(QFileInfo(target).absoluteFilePath());
    if (targetDir.exists()) {
        if (!targetDir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty()) {
            std::cerr << qPrintable(QString::fromLatin1("Target directory \"%1\" already exists and "
                "is not empty.").arg(QDir::toNativeSeparators(targetDir.path()))) << std::endl;
            return EXIT_FAILURE;
        }
    } else {
        if (!QDir().mkpath(targetDir.path())) {
             std::cerr << qPrintable(QString::fromLatin1("Cannot create \"%1\".").arg(
                                         QDir::toNativeSeparators(targetDir.path()))) << std::endl;
            return EXIT_FAILURE;
        }
    }

    QInstaller::CopyDirectoryOperation copyMetadata(nullptr);
    copyMetadata.setArguments(QStringList() << QLatin1String(":/")
        << (targetDir.path() + QLatin1Char('/'))); // Add "/" at the end to make operation work.
    if (!copyMetadata.performOperation()) {
        std::cerr << qPrintable(copyMetadata.errorString()) << std::endl;
        return EXIT_FAILURE;
    }

    if (!targetDir.cd(QLatin1String("metadata"))) {
        std::cerr << qPrintable(QString::fromLatin1("Cannot switch to \"%1/metadata\".")
            .arg(QDir::toNativeSeparators(targetDir.path()))) << std::endl;
        return EXIT_FAILURE;
    }

    int result = EXIT_FAILURE;
    try {
        QFile updatesXml(targetDir.filePath(QLatin1String("Updates.xml")));
        QInstaller::openForRead(&updatesXml);

        QDomDocument doc;
        QDomDocument::ParseResult docResult = doc.setContent(&updatesXml);
        if (!docResult) {
            throw QInstaller::Error(QString::fromLatin1("Cannot read: \"%1\": %2").arg(
                                        QDir::toNativeSeparators(updatesXml.fileName()), docResult.errorMessage));
        }

        QHash<QString, QString> versionMap;
        const QDomElement root = doc.documentElement();
        const QDomNodeList rootChildNodes = root.childNodes();
        for (int i = 0; i < rootChildNodes.count(); ++i) {
            const QDomElement element = rootChildNodes.at(i).toElement();
            if (element.isNull())
                continue;

            QString name, version;
            if (element.tagName() == QLatin1String("PackageUpdate")) {
                const QDomNodeList elementChildNodes = element.childNodes();
                for (int j = 0; j < elementChildNodes.count(); ++j) {
                    const QDomElement e = elementChildNodes.at(j).toElement();
                    if (e.tagName() == QLatin1String("Name"))
                        name = e.text();
                    else if (e.tagName() == QLatin1String("Version"))
                        version = e.text();
                }
                versionMap.insert(name, version);
            }
        }

        foreach (const QString &name, versionMap.keys()) {
            const QInstaller::ResourceCollection c = manager.collectionByName(name.toUtf8());
            if (c.resources().count() <= 0)
                continue;

            if (!targetDir.mkpath(name)) {
                throw QInstaller::Error(QString::fromLatin1("Cannot create target dir: %1.")
                    .arg(targetDir.filePath(name)));
            }

            foreach (const QSharedPointer<QInstaller::Resource> &resource, c.resources()) {
                const bool isOpen = resource->isOpen();
                if ((!isOpen) && (!resource->open()))
                    continue;   // TODO: should we throw here?

                QFile targetFile(targetDir.filePath(name) + QDir::separator()
                    + QString::fromUtf8(resource->name()));
                QInstaller::openForWrite(&targetFile);
                resource->copyData(&targetFile); // copy the 7z files into the target directory

                if (!isOpen) // If we reach that point, either the resource was opened already...
                    resource->close();           // or we did open it and have to close it again.
            }
        }
        result = EXIT_SUCCESS;
    } catch (const QInstaller::Error &error) {
        std::cerr << qPrintable(error.message()) << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception caught." << std::endl;
    }
    return result;
}

void BinaryDump::initializeBinaryDump(qint64 &magicMarker, QList<QInstaller::OperationBlob> &operations,
                                      QInstaller::ResourceCollectionManager &manager,
                                      QVector<QByteArray> &resourceMappings,  quint64 cookie,
                                      const QString path)
{
    QFile file(path);
    QInstaller::openForRead(&file);

    QInstaller::BinaryContent::readBinaryContent(&file, &operations, &manager, &magicMarker, cookie);

    // map the inbuilt resources
    const QInstaller::ResourceCollection meta = manager.collectionByName("QResources");
    foreach (const QSharedPointer<QInstaller::Resource> &resource, meta.resources()) {
        const bool isOpen = resource->isOpen();
        if ((!isOpen) && (!resource->open()))
            continue;

        const QByteArray ba = resource->readAll();
        if (!QResource::registerResource((const uchar*) ba.data(), QLatin1String(":/metadata")))
            throw QInstaller::Error(QLatin1String("Cannot register in-binary resource."));
        resourceMappings.append(ba);
        if (!isOpen)
            resource->close();
    }
}
