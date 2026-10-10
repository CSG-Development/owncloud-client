/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#ifndef TESTREPOSITORY_H
#define TESTREPOSITORY_H

#include "downloadfiletask.h"
#include "job.h"
#include "repository.h"

#include <QFutureWatcher>
#include <QTimer>

namespace QInstaller {

class PackageManagerCore;

class INSTALLER_EXPORT TestRepository : public Job
{
    Q_OBJECT
    Q_DISABLE_COPY(TestRepository)

public:
    explicit TestRepository(PackageManagerCore *parent = 0);
    ~TestRepository();

    Repository repository() const;
    void setRepository(const Repository &repository);

private slots:
    void doStart() override;
    void doCancel() override;

    void onTimeout();
    void downloadCompleted();

private:
    void reset();

private:
    PackageManagerCore *m_core;

    QTimer m_timer;
    Repository m_repository;
    QFutureWatcher<FileTaskResult> m_xmlTask;
};

} // namespace QInstaller

#endif  // TESTREPOSITORY_H
