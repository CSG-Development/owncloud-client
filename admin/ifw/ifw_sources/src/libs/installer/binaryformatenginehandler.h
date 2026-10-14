/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef BINARYFORMATENGINEHANDLER_H
#define BINARYFORMATENGINEHANDLER_H

#include "binaryformat.h"

#include <QtCore/private/qabstractfileengine_p.h>

namespace QInstaller {

class INSTALLER_EXPORT BinaryFormatEngineHandler : public QAbstractFileEngineHandler
{
    Q_DISABLE_COPY(BinaryFormatEngineHandler)

public:
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    std::unique_ptr<QAbstractFileEngine> create(const QString &fileName) const;
#else
    QAbstractFileEngine *create(const QString &fileName) const;
#endif
    void clear();
    static BinaryFormatEngineHandler *instance();

    void registerResources(const QList<ResourceCollection> &collections);
    void registerResource(const QString &fileName, const QString &resourcePath);

private:
    BinaryFormatEngineHandler() {}
    ~BinaryFormatEngineHandler() {}

private:
    QHash<QByteArray, ResourceCollection> m_resources;
};

} // namespace QInstaller

#endif // BINARYFORMATENGINEHANDLER_H
