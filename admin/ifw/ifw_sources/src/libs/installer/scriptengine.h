/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include "installer_global.h"

#include <QJSValue>
#include <QJSEngine>

namespace QInstaller {

class PackageManagerCore;
class GuiProxy;

class INSTALLER_EXPORT ScriptEngine : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ScriptEngine)

public:
    explicit ScriptEngine(PackageManagerCore *core = 0);

    QJSValue globalObject() const { return m_engine.globalObject(); }
    QJSValue newQObject(QObject *object, bool qtScriptCompat = true);
    QJSValue newArray(uint length = 0);
    QJSValue evaluate(const QString &program, const QString &fileName = QString(),
        int lineNumber = 1);

    void addToGlobalObject(QObject *object);
    void removeFromGlobalObject(QObject *object);

    QJSValue loadInContext(const QString &context, const QString &fileName,
        const QString &scriptInjection = QString());
    QJSValue callScriptMethod(const QJSValue &context, const QString &methodName,
        const QJSValueList &arguments = QJSValueList());

private slots:
    void setGuiQObject(QObject *guiQObject);

private:
    QJSValue generateMessageBoxObject();
    QJSValue generateQInstallerObject();
    QJSValue generateWizardButtonsObject();
    QJSValue generateDesktopServicesObject();
#ifdef Q_OS_WIN
    QJSValue generateSettingsObject();
#endif

private:
    QJSEngine m_engine;
    QHash<QString, QStringList> m_callstack;
    GuiProxy *m_guiProxy;
    PackageManagerCore *m_core;
};

}
Q_DECLARE_METATYPE(QInstaller::ScriptEngine*)

#endif // SCRIPTENGINE_H
