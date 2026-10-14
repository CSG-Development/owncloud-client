/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef INSTALLERBASE_H
#define INSTALLERBASE_H

#include "sdkapp.h"
#include "commandlineparser.h"

namespace QInstaller {
    class PackageManagerCore;
}

class InstallerBase : public SDKApp<QApplication>
{
    Q_OBJECT
    Q_DISABLE_COPY(InstallerBase)

public:
    InstallerBase(int &argc, char *argv[]);
    ~InstallerBase();

    int run();

private:
    void dumpResourceTree() const;
    QStringList repositories(const QString &list) const;
};

#endif // INSTALLERBASE_H
