/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef PRODUCTKEYCHECK_H
#define PRODUCTKEYCHECK_H

#include "installer_global.h"

#include <QString>
#include <QUiLoader>
#include <QStandardItem>

namespace QInstaller {

class PackageManagerCore;
class Repository;

} // QInstaller

class INSTALLER_EXPORT ProductKeyCheck
{
    Q_DISABLE_COPY(ProductKeyCheck)

public:
    ~ProductKeyCheck();
    static ProductKeyCheck *instance();
    void init(QInstaller::PackageManagerCore *core);

    QUiLoader *uiLoader();

    // was validLicense
    bool hasValidKey();
    QString lastErrorString();
    QString maintainanceToolDetailErrorNotice();

    bool applyKey(const QStringList &arguments);

    // to filter none valid licenses
    bool isValidLicenseTextFile(const QString &fileName);

    // to filter repositories not matching the license
    bool isValidRepository(const QInstaller::Repository &repository) const;

    void addPackagesFromXml(const QString &xmlPath);
    bool isValidPackage(const QString &packageName) const;

    QList<int> registeredPages() const;
    bool hasValidLicense() const;
    bool hasAcceptedAllLicenses() const;
    QString licenseAcceptanceText() const;
    QString securityWarning() const;
    QString additionalMetaDownloadWarning() const;
    QString additionalAcceptanceForCategory(const QStandardItem *item) const;

private:
    ProductKeyCheck();
    class ProductKeyCheckPrivate *const d;
};

#endif // PRODUCTKEYCHECK_H
