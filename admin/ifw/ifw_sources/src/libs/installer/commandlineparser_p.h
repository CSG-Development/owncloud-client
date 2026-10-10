/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef COMMANDLINEPARSER_P_H
#define COMMANDLINEPARSER_P_H

#include "commandlineparser.h"

#include <QCommandLineParser>

class CommandLineParser;

class CommandLineParserPrivate
{
    Q_DISABLE_COPY(CommandLineParserPrivate)

public:
    explicit CommandLineParserPrivate(CommandLineParser *parser) : q(parser) {};

    void addExtensionsOptions() const;

private:
    CommandLineParser *const q;
};

#endif // COMMANDLINEPARSER_P_H
