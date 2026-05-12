#pragma once

#include <QRegularExpression>
#include <QString>

namespace APP::Wizard {

inline bool isValidEmailAddress(const QString &emailText)
{
    if (emailText.trimmed().isEmpty()) {
        return false;
    }

    static const QRegularExpression rx(
        QStringLiteral("^[0-9a-zA-Z]+([0-9a-zA-Z]*[-._+])*[0-9a-zA-Z]+@[0-9a-zA-Z]+([-.][0-9a-zA-Z]+)*([0-9a-zA-Z]*[.])[a-zA-Z]{2,6}$"),
        QRegularExpression::CaseInsensitiveOption);
    return rx.match(emailText).hasMatch();
}

}
