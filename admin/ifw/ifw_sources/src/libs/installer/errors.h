/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef ERRORS_H
#define ERRORS_H

#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtCore/QException>

namespace QInstaller {

class Error : public QException
{
public:
    Error() = default;

    explicit Error(const QString &message)
        : m_message(message)
    {}

    void raise() const override { throw *this; }
    Error *clone() const override { return new Error(*this); }

    QString message() const { return m_message; }

private:
    QString m_message;
};

}

#endif // ERRORS_H
