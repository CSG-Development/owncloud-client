/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#include <QTextDocument>
#include <QMetaEnum>

#include "globals.h"

#if defined(Q_OS_MACOS) || defined(Q_OS_LINUX)
#include <termios.h>
#include <unistd.h>
#elif defined(Q_OS_WIN)
#include <conio.h>
#endif
#include <iostream>

const char IFW_SERVER[] = "ifw.server";
const char IFW_INSTALLER_INSTALLLOG[] = "ifw.installer.installlog";
const char IFW_DEVELOPER_BUILD[] = "ifw.developer.build";

// Internal-only, hidden in --help text
const char IFW_PROGRESS_INDICATOR[] = "ifw.progress.indicator";

static const QStringList scReservedCharsInComponentName = QStringList() << QLatin1String(":") << QLatin1String("-");

namespace QInstaller
{

/*!
    \fn QInstaller::lcServer()
    \internal
*/

/*!
    \fn QInstaller::lcInstallerInstallLog()
    \internal
*/

/*!
    \fn QInstaller::lcProgressIndicator()
    \internal
*/

/*!
    \fn QInstaller::lcDeveloperBuild()
    \internal
*/

/*!
    \fn inline QInstaller::splitStringWithComma(const QString &value())
    Splits \a value into substrings wherever comma occurs, and returns the list of those strings.
    \internal
*/

Q_LOGGING_CATEGORY(lcServer, IFW_SERVER)
Q_LOGGING_CATEGORY(lcInstallerInstallLog, IFW_INSTALLER_INSTALLLOG)
Q_LOGGING_CATEGORY(lcProgressIndicator, IFW_PROGRESS_INDICATOR)
Q_LOGGING_CATEGORY(lcDeveloperBuild, IFW_DEVELOPER_BUILD)

/*!
    Returns available logging categories.
*/
QStringList loggingCategories()
{
    static QStringList categories = QStringList()
            << QLatin1String(IFW_INSTALLER_INSTALLLOG)
            << QLatin1String(IFW_SERVER)
            << QLatin1String(IFW_DEVELOPER_BUILD)
            << QLatin1String("js");
    return categories;

}

Q_GLOBAL_STATIC_WITH_ARGS(QRegularExpression, staticCommaRegExp, (QLatin1String("(, |,)")));

/*!
    \internal
*/
QRegularExpression commaRegExp()
{
    return *staticCommaRegExp();
}

/*!
    Converts and returns a string \a html containing an HTML document as a plain text.
*/
QString htmlToString(const QString &html)
{
    QTextDocument doc;
    doc.setHtml(html);
    return doc.toPlainText();
}

/*!
    \internal
*/
QString enumToString(const QMetaObject& metaObject, const char *enumerator, int key)
{
    QString value = QString();
    int enumIndex = metaObject.indexOfEnumerator(enumerator);
    if (enumIndex != -1) {
        QMetaEnum en = metaObject.enumerator(enumIndex);
        value = QLatin1String(en.valueToKey(key));
    }
    return value;
}

void askForCredentials(QString *username, QString *password, const QString &usernameTitle, const QString &passwordTitle)
{
    std::string usernameStdStr;
    std::string passwordStdStr;

    std::cout << qPrintable(usernameTitle);
    std::cin >> usernameStdStr;

    std::cout << qPrintable(passwordTitle);
#if defined(Q_OS_MACOS) || defined(Q_OS_LINUX)
    termios oldTerm;
    termios term;

    // Turn off echoing
    tcgetattr(STDIN_FILENO, &oldTerm);
    term = oldTerm;
    term.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    std::cin >> passwordStdStr;

    // Clear input buffer
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Restore old attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &oldTerm);
#elif defined(Q_OS_WIN)
    char ch;
    while ((ch = _getch()) != '\r') { // Return key
        if (ch == '\b') { // Backspace key
            if (!passwordStdStr.empty())
                passwordStdStr.pop_back();
        } else {
            passwordStdStr.push_back(ch);
        }
    }
    // Clear input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

#endif
    std::cout << "\n";

    *username = username->fromStdString(usernameStdStr);
    *password = password->fromStdString(passwordStdStr);
}

bool containsReservedCharacters(QString *message, const QStringList &packageEntryList)
{
    for (const QString &entry : std::as_const(packageEntryList)) {
        for (const QString &reservedCharacter : std::as_const(scReservedCharsInComponentName)) {
            if (!entry.contains(reservedCharacter))
                continue;
            *message = QLatin1String("Error: The package directory '%1' contains reserved character '%2'. "
                "The character is reserved for separating the version number from the "
                "identifier. ").arg(entry, reservedCharacter);
            return true;
        }
    }
    return false;
}

} // namespace QInstaller

