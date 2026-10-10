/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef EXTRACTARCHIVEOPERATION_H
#define EXTRACTARCHIVEOPERATION_H

#include "qinstallerglobal.h"

#include <QtCore/QObject>

namespace QInstaller {

class INSTALLER_EXPORT ExtractArchiveOperation : public QObject, public Operation
{
    Q_OBJECT
    friend class WorkerThread;

public:
    explicit ExtractArchiveOperation(PackageManagerCore *core);

    void backup() override;
    bool performOperation() override;
    bool undoOperation() override;
    bool testOperation() override;

    quint64 sizeHint() override;

    bool readDataFileContents(QString &targetDir, QStringList *resultList);

Q_SIGNALS:
    void outputTextChanged(const QString &progress);
    void progressChanged(double);

private:
    void startUndoProcess(const QStringList &files);
    void deleteDataFile(const QString &fileName);

    QString generateBackupName(const QString &fn);
    bool prepareForFile(const QString &filename);
    void addParentFolders(const QString &componentDirectory, QStringList &files);

private:
    typedef QPair<QString, QString> Backup;
    typedef QVector<Backup> BackupFiles;

    class Callback;
    class Worker;
    class Receiver;

private:
    QString m_relocatedDataFileName;
    BackupFiles m_backupFiles;
    quint64 m_totalEntries;
};

}

#endif
