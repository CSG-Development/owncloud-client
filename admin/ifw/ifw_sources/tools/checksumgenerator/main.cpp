/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "utils.h"
#include "fileio.h"
#include "errors.h"

#include <QFile>
#include <QDirIterator>
#include <iostream>

void createHashFile(QFile &archiveFile)
{
    QFile archiveHashFile(archiveFile.fileName() + QLatin1String(".sha1"));
    if (archiveHashFile.exists())
        return;

    try {
        QInstaller::openForRead(&archiveFile);
        const QByteArray hashOfArchiveData = QInstaller::calculateHash(&archiveFile,
                                                                       QCryptographicHash::Sha1).toHex();
        archiveFile.close();

        QInstaller::openForWrite(&archiveHashFile);
        archiveHashFile.write(hashOfArchiveData);
        qDebug() << "Generated sha1 hash:" << hashOfArchiveData << " for file " << archiveHashFile.fileName();
        archiveHashFile.close();
    } catch (const QInstaller::Error &/*e*/) {
        archiveFile.close();
        archiveHashFile.close();
        throw;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments().mid(1);
    for (QStringList::const_iterator it = args.begin(); it != args.end(); ++it) {
        if (*it == QLatin1String("--sourceDir")) {
            ++it;
            if (it == args.end())
                std::cerr << qPrintable(QString::fromLatin1("Error: Parameter missing argument.")) << std::endl << std::endl;

            const QString sourceDir = *it;
            //tar|tar.gz|tar.bz2|tar.xz|zip|7z
            QDirIterator it(sourceDir, QStringList(QLatin1String("*content.7z"))
                                                << QLatin1String("*content.tar")
                                                << QLatin1String("*content.tar.gz")
                                                << QLatin1String("*content.tar.bz2")
                                                << QLatin1String("*content.tar.xz")
                                                << QLatin1String("*content.zip"), QDir::Files | QDir::CaseSensitive, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                QFile file(it.next());
                createHashFile(file);
                qDebug() << file.fileName();
            }
        } else if (*it == QLatin1String("--sourceFile")) {
            ++it;
            if (it == args.end())
                std::cerr << qPrintable(QString::fromLatin1("Error: Parameter missing argument.")) << std::endl << std::endl;

            QFile archiveFile(*it);
            createHashFile(archiveFile);
        }
    }
    return EXIT_SUCCESS;
}

