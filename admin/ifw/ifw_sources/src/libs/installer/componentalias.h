/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef COMPONENTALIAS_H
#define COMPONENTALIAS_H

#include "installer_global.h"

#include <QHash>
#include <QMetaEnum>
#include <QString>
#include <QSet>

namespace QInstaller {

class Component;
class ComponentAlias;
class PackageManagerCore;

struct INSTALLER_EXPORT AliasSource
{
    enum class SourceFileFormat {
        Unknown = -1,
        Xml = 0,
        Json
    };

    AliasSource();
    AliasSource(SourceFileFormat aFormat, const QString &aFilename, int aPriority);
    AliasSource(const AliasSource &other);

    SourceFileFormat format;
    QString filename;
    int priority;
};

INSTALLER_EXPORT hashValue qHash(const AliasSource &key, hashValue seed);
INSTALLER_EXPORT bool operator==(const AliasSource &lhs, const AliasSource &rhs);

class INSTALLER_EXPORT AliasFinder
{
public:
    using AliasData = QVariantMap;
    using AliasDataHash = QMultiHash<QString, AliasData>;

    enum struct Resolution {
        AddNew,
        KeepExisting,
        RemoveExisting
    };

    explicit AliasFinder(PackageManagerCore *core);
    ~AliasFinder();

    bool run();
    QList<ComponentAlias *> aliases() const;

    void setAliasSources(const QSet<AliasSource> &sources);

private:
    void clear();
    Resolution checkPriorityAndVersion(const AliasData &data) const;

    bool parseXml(AliasSource source);
    bool parseJson(AliasSource source);

private:
    PackageManagerCore *const m_core;

    QSet<AliasSource> m_sources;
    AliasDataHash m_aliasData;
    QHash<QString, ComponentAlias *> m_aliases;
};

class INSTALLER_EXPORT ComponentAlias : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ComponentAlias)

public:
    enum UnstableError {
        ReferenceToUnstable = 0,
        MissingComponent,
        UnselectableComponent,
        MissingAlias,
        ComponentNameConfict
    };
    Q_ENUM(UnstableError)

    ComponentAlias(PackageManagerCore *core);
    ~ComponentAlias();

    QString name() const;
    QString displayName() const;
    QString description() const;

    QString version() const;

    bool isVirtual() const;

    bool isSelected() const;
    void setSelected(bool selected);

    QList<Component *> components();
    QList<ComponentAlias *> aliases();

    QString value(const QString &key, const QString &defaultValue = QString()) const;
    void setValue(const QString &key, const QString &value);
    QStringList keys() const;

    bool isUnstable() const;
    void setUnstable(UnstableError error, const QString &message = QString());
    QString componentErrorMessage() const;
    bool missingOptionalComponents() const;

private:
    void addRequiredAliases(const QStringList &aliases, const bool optional);
    void addRequiredComponents(const QStringList &components, const bool optional);

private:
    PackageManagerCore *const m_core;

    QHash<QString, QString> m_variables;

    bool m_selected;
    bool m_unstable;

    QList<Component *> m_components;
    bool m_missingOptionalComponents;
    QList<ComponentAlias *> m_aliases;
    QString m_componentErrorMessages;
};

} // namespace QInstaller

Q_DECLARE_METATYPE(QInstaller::ComponentAlias *)
Q_DECLARE_METATYPE(QInstaller::AliasSource);

#endif // COMPONENTALIAS_H
