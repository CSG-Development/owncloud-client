/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "kdtoolsglobal.h"

#include <QString>
#include <QHash>

QT_BEGIN_NAMESPACE
class QProcess;
class QProcessEnvironment;
QT_END_NAMESPACE

namespace KDUpdater {

class KDTOOLS_EXPORT Environment
{
public:
    static Environment &instance();

    ~Environment() {}

    QString value(const QString &key, const QString &defaultValue = QString()) const;
    void setTemporaryValue(const QString &key, const QString &value);

    QProcessEnvironment applyTo(const QProcessEnvironment &qpe) const;
    void applyTo(QProcess *process);

private:
    Environment() {}

private:
    Q_DISABLE_COPY(Environment)
    QHash<QString, QString> m_tempValues;
};

} // namespace KDUpdater

#endif // ENVIRONMENT_H
