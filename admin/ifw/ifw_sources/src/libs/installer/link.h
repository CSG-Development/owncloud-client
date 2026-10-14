/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef LINK_H
#define LINK_H

#include <QString>

class Link
{
public:
    explicit Link(const QString &path);
    static Link create(const QString &link, const QString &targetPath);
    QString targetPath() const;
    bool targetExists();
    bool exists();
    bool isValid();
    bool remove();

private:
    QString m_path;
};

#endif // LINK_H
