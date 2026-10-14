/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef UPDATEFINDER_H
#define UPDATEFINDER_H

#include "task.h"
#include "packagesource.h"
#include "updatesinfo_p.h"
#include "abstracttask.h"

#include <memory>

using namespace QInstaller;
namespace KDUpdater {

class LocalPackageHub;
class Update;

class ParseXmlFilesTask : public AbstractTask<void>
{
    Q_OBJECT
    Q_DISABLE_COPY(ParseXmlFilesTask)

public:
    ParseXmlFilesTask(UpdatesInfo *info)
        : m_info(info)
    {}

    void doTask(QFutureInterface<void> &fi) override
    {
        fi.reportStarted();
        fi.setExpectedResultCount(1);

        if (fi.isCanceled()) {
            fi.reportFinished();
            return; // ignore already canceled
        }
        m_info->parseFile();

        fi.reportFinished();
    }

private:
    UpdatesInfo *const m_info;
};


class KDTOOLS_EXPORT UpdateFinder : public Task
{
    Q_OBJECT
    class Private;

    enum struct Resolution {
        AddPackage,
        KeepExisting,
        RemoveExisting
    };

public:
    UpdateFinder();
    ~UpdateFinder();

    QList<Update *> updates() const;

    void setLocalPackageHub(std::weak_ptr<LocalPackageHub> hub);
    void setPackageSources(const QSet<QInstaller::PackageSource> &sources);

private:
    void doRun() override;
    bool doStop() override;
    bool doPause() override;
    bool doResume() override;
    void clear();
    void computeUpdates();
    void cancelComputeUpdates();
    void createUpdatesInfoList();
    bool parseUpdateXMLFiles();
    bool removeInvalidObjects();
    bool computeApplicableUpdates();

    QList<UpdateInfo> applicableUpdates(UpdatesInfo *updatesInfo);
    void createUpdateObjects(const PackageSource &source, const QList<UpdateInfo> &updateInfoList);
    Resolution checkPriorityAndVersion(const QInstaller::PackageSource &source, const QVariantHash &data) const;
    bool waitForJobToFinish(const int &currentCount, const int &totalsCount);

private slots:
    void parseUpdatesXmlTaskFinished();

private:
    QSet<PackageSource> m_packageSources;
    std::weak_ptr<LocalPackageHub> m_localPackageHub;
    QHash<QString, Update *> m_updates;

    bool m_cancel;
    QHash<UpdatesInfo *, QInstaller::PackageSource> m_updatesInfoList;
    int m_updatesXmlTasks;
    int m_updatesXmlTasksToComplete;
    QList<ParseXmlFilesTask*> m_xmlFileTasks;
};

} // namespace KDUpdater

#endif // UPDATEFINDER_H
