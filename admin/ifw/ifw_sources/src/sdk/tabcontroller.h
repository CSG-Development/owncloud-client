/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef TABCONTROLLER_H
#define TABCONTROLLER_H

#include <QtCore/QHash>
#include <QtCore/QObject>

namespace QInstaller {
    class PackageManagerGui;
    class PackageManagerCore;
    class Settings;
}

class IntroductionPageImpl;

class TabController : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TabController)

public:
    explicit TabController(QObject *parent = 0);
    ~TabController();

    void setGui(QInstaller::PackageManagerGui *gui);
    void setManager(QInstaller::PackageManagerCore *core);

    void setControlScript(const QString &script);

public Q_SLOTS:
    int init();

private Q_SLOTS:
    void restartWizard();
    void onSettingsButtonClicked();
    void onAboutApplicationClicked();
    void onClearCacheClicked();
    void onCurrentIdChanged(int newId);
    void onNetworkSettingsChanged(const QInstaller::Settings &settings);

private:
    class Private;
    Private *const d;
};

#endif // TABCONTROLLER_H
