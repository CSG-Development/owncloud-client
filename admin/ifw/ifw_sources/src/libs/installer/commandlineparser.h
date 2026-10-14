/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QCommandLineParser>
#include <QHash>

class CommandLineParserPrivate;

class CommandLineParser
{
public:
    enum OptionContextFlag {
        CommandLineOnly = 0x1,
        NoEchoValue = 0x2
    };
    Q_DECLARE_FLAGS(OptionContextFlags, OptionContextFlag)

    CommandLineParser();
    ~CommandLineParser();

    bool addOption(const QCommandLineOption &option) { return m_parser.addOption(option); }
    bool addOptionWithContext(const QCommandLineOption &option, OptionContextFlags flags);

    QString helpText() const { return m_parser.helpText(); }
    bool isSet(const QString &option) const { return m_parser.isSet(option); }
    QStringList unknownOptionNames() const { return m_parser.unknownOptionNames(); }
    QStringList positionalArguments() const { return m_parser.positionalArguments(); }
    bool parse(const QStringList &argumens) { return m_parser.parse(argumens); }
    QString value(const QString &option) const { return m_parser.value(option); }
    QStringList optionNames() const { return m_parser.optionNames(); }

    OptionContextFlags optionContextFlags(const QString &option) const;
    QStringList arguments() const;

private:
    QCommandLineParser m_parser;
    CommandLineParserPrivate *const d;

    QHash<QString, OptionContextFlags> m_optionContextFlagsNameHash;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(CommandLineParser::OptionContextFlags)

#endif // COMMANDLINEPARSER_H
