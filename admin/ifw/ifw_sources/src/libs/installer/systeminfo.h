/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include <QObject>

namespace QInstaller {

class SystemInfo : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SystemInfo)

    Q_PROPERTY(QString currentCpuArchitecture READ currentCpuArchitecture CONSTANT)
    Q_PROPERTY(QString buildCpuArchitecture READ buildCpuArchitecture CONSTANT)
    Q_PROPERTY(QString kernelType READ kernelType CONSTANT)
    Q_PROPERTY(QString kernelVersion READ kernelVersion CONSTANT)
    Q_PROPERTY(QString productType READ productType CONSTANT)
    Q_PROPERTY(QString productVersion READ productVersion CONSTANT)
    Q_PROPERTY(QString prettyProductName READ prettyProductName CONSTANT)

public:
    explicit SystemInfo(QObject *parent = 0);

    QString currentCpuArchitecture() const;
    QString buildCpuArchitecture() const;
    QString kernelType() const;
    QString kernelVersion() const;
    QString productType() const;
    QString productVersion() const;
    QString prettyProductName() const;
};

} // namespace QInstaller

#endif // SYSTEMINFO_H
