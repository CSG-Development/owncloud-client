/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef PACKAGEMANAGERCOREDATA_H
#define PACKAGEMANAGERCOREDATA_H

#include "settings.h"
#include <QSettings>

namespace QInstaller {

class PackageManagerCoreData
{
public:
    PackageManagerCoreData() {}
    explicit PackageManagerCoreData(const QHash<QString, QString> &variables, const bool isInstaller);

    void clear();
    void addDynamicPredefinedVariables();
    void setUserDefinedVariables(const QHash<QString, QString> &variables);
    void addNewVariable(const QString &key, const QString &value);

    Settings &settings() const;
    QStringList keys() const;

    inline QString settingsFilePath() {
        return m_settingsFilePath;
    }

    bool contains(const QString &key) const;
    bool setValue(const QString &key, const QString &normalizedValue);
    QVariant value(const QString &key, const QVariant &_default = QVariant(), const QSettings::Format &format = QSettings::NativeFormat) const;
    QString key(const QString &value) const;

    QString replaceVariables(const QString &str) const;
    QByteArray replaceVariables(const QByteArray &ba) const;

private:
    mutable Settings m_settings;
    QString m_settingsFilePath;
    QHash<QString, QString> m_variables;
};

}   // namespace QInstaller

#endif  // PACKAGEMANAGERCOREDATA_H
