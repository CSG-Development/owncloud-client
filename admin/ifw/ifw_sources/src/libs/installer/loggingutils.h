/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef LOGGINGUTILS_H
#define LOGGINGUTILS_H

#include "qinstallerglobal.h"
#include "localpackagehub.h"

#include <QObject>
#include <QIODevice>
#include <QTextStream>
#include <QBuffer>
#include <QMutex>

namespace QInstaller {

class Component;
class ComponentAlias;

class INSTALLER_EXPORT LoggingHandler
{
    Q_DISABLE_COPY(LoggingHandler)
    Q_ENUMS(VerbosityLevel)

public:
    enum VerbosityLevel {
        Silent = 0,
        Normal = 1,
        Detailed = 2,
        Minimum = Silent,
        Maximum = Detailed
    };

    static LoggingHandler &instance();
    void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    void setVerbose(bool v);
    bool isVerbose() const;
    VerbosityLevel verboseLevel() const;
    bool outputRedirected() const;

    void printUpdateInformation(const QList<Component *> &components) const;
    void printLocalPackageInformation(const QList<KDUpdater::LocalPackage> &packages) const;
    void printPackageInformation(const PackagesList &matchedPackages, const LocalPackagesMap &installedPackages) const;
    void printAliasInformation(const QList<ComponentAlias *> &aliases);

    friend VerbosityLevel &operator++(VerbosityLevel &level, int);
    friend VerbosityLevel &operator--(VerbosityLevel &level, int);

private:
    LoggingHandler();
    ~LoggingHandler();

    QString trimAndPrepend(QtMsgType type, const QString &msg) const;

private:
    VerbosityLevel m_verbLevel;
    bool m_outputRedirected;

    QMutex m_mutex;
};

class INSTALLER_EXPORT VerboseWriterOutput
{
public:
    virtual bool write(const QString &fileName, QIODevice::OpenMode openMode, const QByteArray &data) = 0;

protected:
    ~VerboseWriterOutput();
};

class INSTALLER_EXPORT PlainVerboseWriterOutput : public VerboseWriterOutput
{
public:
    virtual bool write(const QString &fileName, QIODevice::OpenMode openMode, const QByteArray &data) override;
};

class INSTALLER_EXPORT VerboseWriterAdminOutput : public VerboseWriterOutput
{
public:
    VerboseWriterAdminOutput(PackageManagerCore *core) : m_core(core) {}

    virtual bool write(const QString &fileName, QIODevice::OpenMode openMode, const QByteArray &data) override;

private:
    PackageManagerCore *m_core;
};

class INSTALLER_EXPORT VerboseWriter
{
public:
    VerboseWriter();
    ~VerboseWriter();

    static VerboseWriter *instance();

    bool flush(VerboseWriterOutput *output);

    void appendLine(const QString &msg);
    void setFileName(const QString &fileName);

private:
    QTextStream m_stream;
    QBuffer m_preFileBuffer;
    QString m_logFileName;
    QString m_currentDateTimeAsString;
};

LoggingHandler::VerbosityLevel &operator++(LoggingHandler::VerbosityLevel &level, int);
LoggingHandler::VerbosityLevel &operator--(LoggingHandler::VerbosityLevel &level, int);

} // namespace QInstaller

#endif // LOGGINGUTILS_H
