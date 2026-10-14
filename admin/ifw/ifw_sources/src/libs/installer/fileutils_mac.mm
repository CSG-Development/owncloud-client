/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "fileutils.h"

#include "errors.h"

#include <QCoreApplication>

#include <objc/objc.h>
#include <Foundation/NSURL.h>
#include <Foundation/NSError.h>>

namespace QInstaller {

/*!
    \internal

    Creates a bookmark variant of Finder alias from target \a path to \a alias.
    Throws \c Error on failure.
*/
void mkalias(const QString &path, const QString &alias)
{
    NSURL *targetUrl = [NSURL fileURLWithPath:path.toNSString()];
    NSURL *aliasUrl = [NSURL fileURLWithPath:alias.toNSString()];

    NSError *error = nil;
    NSData *data = [targetUrl bookmarkDataWithOptions:NSURLBookmarkCreationSuitableForBookmarkFile
        includingResourceValuesForKeys:nil relativeToURL:nil error:&error];

    if (data != nil) {
        BOOL success = [NSURL writeBookmarkData:data toURL:aliasUrl
            options:NSURLBookmarkCreationSuitableForBookmarkFile error:&error];
        if (success == NO) {
            throw Error(QCoreApplication::translate("QInstaller",
                "Cannot create alias from \"%1\" to \"%2\": %3.")
                    .arg(path, alias, QString::fromNSString(error.localizedDescription))
            );
        }
    } else {
        throw Error(QCoreApplication::translate("QInstaller",
            "Could not get bookmark from URL \"%1\": %2.").arg(
                QString::fromNSString(targetUrl.absoluteString),
                QString::fromNSString(error.localizedDescription)
            )
        );
    }
}

} // namespace QInstaller
