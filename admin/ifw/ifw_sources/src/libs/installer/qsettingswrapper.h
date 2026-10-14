/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef QSETTINGSWRAPPER_H
#define QSETTINGSWRAPPER_H

#include "protocol.h"
#include "remoteobject.h"

#include <QVariant>

#include <QSettings>

QT_FORWARD_DECLARE_CLASS(QTextCodec)

namespace QInstaller {

class INSTALLER_EXPORT QSettingsWrapper : public RemoteObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QSettingsWrapper)

public:
    enum Status {
        NoError = 0,
        AccessError,
        FormatError
    };

    enum Scope {
        UserScope,
        SystemScope
    };

    explicit QSettingsWrapper(const QString &organization,
        const QString &application = QString(), QObject *parent = 0);
    QSettingsWrapper(Scope scope, const QString &organization,
        const QString &application = QString(), QObject *parent = 0);
    QSettingsWrapper(QSettings::Format format, Scope scope, const QString &organization,
        const QString &application = QString(), QObject *parent = 0);
    QSettingsWrapper(const QString &fileName, QSettings::Format format, QObject *parent = 0);
    ~QSettingsWrapper();

    void clear();
    void sync();
    Status status() const;

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
    void beginGroup(const QString &prefix);
#else
    void beginGroup(QAnyStringView prefix);
#endif
    void endGroup();
    QString group() const;

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
    int beginReadArray(const QString &prefix);
    void beginWriteArray(const QString &prefix, int size = -1);
#else
    int beginReadArray(QAnyStringView prefix);
    void beginWriteArray(QAnyStringView prefix, int size = -1);
#endif
    void endArray();
    void setArrayIndex(int i);

    QStringList allKeys() const;
    QStringList childKeys() const;
    QStringList childGroups() const;
    bool isWritable() const;

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;
#else
    void setValue(QAnyStringView key, const QVariant &value);
    QVariant value(QAnyStringView key, const QVariant &defaultValue) const;
    QVariant value(QAnyStringView key) const;
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
    void remove(const QString &key);
    bool contains(const QString &key) const;
#else
    void remove(QAnyStringView key);
    bool contains(QAnyStringView key) const;
#endif
    void setFallbacksEnabled(bool b);
    bool fallbacksEnabled() const;

    QString fileName() const;
    QSettings::Format format() const;
    Scope scope() const;
    QString organizationName() const;
    QString applicationName() const;

private:
    bool createSocket() const;

private: // we cannot support the following functionality
    explicit QSettingsWrapper(QObject *parent = 0)
        : RemoteObject(QLatin1String(Protocol::QSettings), parent)
    {}

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
    void setIniCodec(QTextCodec * /*codec*/);
    void setIniCodec(const char * /*codecName*/);
    QTextCodec *iniCodec() const { return 0; }
#endif
    static void setDefaultFormat(QSettings::Format /*format*/);
    static QSettings::Format defaultFormat() { return QSettings::NativeFormat; }
    static void setPath(QSettings::Format /*format*/, Scope /*scope*/, const QString & /*path*/);

    typedef QMap<QString, QVariant> SettingsMap;
    typedef bool(*ReadFunc)(QIODevice &device, SettingsMap &map);
    typedef bool(*WriteFunc)(QIODevice &device, const SettingsMap &map);

    static QSettings::Format registerFormat(const QString &extension, ReadFunc readFunc, WriteFunc writeFunc,
        Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive)
    {
        Q_UNUSED(extension)
        Q_UNUSED(readFunc)
        Q_UNUSED(writeFunc)
        Q_UNUSED(caseSensitivity)
        return QSettings::NativeFormat;
    }

private:
    class Private;
    Private *d;
};

} // namespace QInstaller

#endif  // QSETTINGSWRAPPER_H
