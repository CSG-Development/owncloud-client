/* Copyright C 2002 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef UPDATESINFO_P_H
#define UPDATESINFO_P_H

#include "updater.h"
#include "updatesinfodata_p.h"

#include <QHash>
#include <QSharedData>
#include <QVariant>

// They are not a part of the public API
// Classes and structures in this header file are for internal use only but still exported for auto tests.

namespace KDUpdater {

struct KDTOOLS_EXPORT UpdateInfo
{
    QHash<QString, QVariant> data;
};

class KDTOOLS_EXPORT UpdatesInfo
{
public:
    enum Error
    {
        NoError = 0,
        NotYetReadError,
        CouldNotReadUpdateInfoFileError,
        InvalidXmlError,
        InvalidContentError
    };

    UpdatesInfo(const bool postLoadComponentScript = false);
    ~UpdatesInfo();

    bool isValid() const;

    Error error() const;
    QString errorString() const;

    QString fileName() const;
    void setFileName(const QString &updateXmlFile);
    void parseFile();

    QString applicationName() const;
    QString applicationVersion() const;

    int updateInfoCount() const;
    UpdateInfo updateInfo(int index) const;
    QList<UpdateInfo> updatesInfo() const;

private:
    QSharedDataPointer<UpdatesInfoData> d;
};

} // namespace KDUpdater

#endif // UPDATESINFO_P_H
