/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef QINSTALLER_MESSAGEBOXHANDLER_H
#define QINSTALLER_MESSAGEBOXHANDLER_H

#include <installer_global.h>

#include <QHash>
#include <QMessageBox>
#include <QObject>

namespace QInstaller {

class INSTALLER_EXPORT MessageBoxHandler : public QObject
{
    Q_OBJECT

public:
    enum DefaultAction {
        AskUser,
        Accept,
        Reject,
        Default
    };

    enum MessageType{
        criticalType,
        informationType,
        questionType,
        warningType
    };

    static MessageBoxHandler *instance();
    static QWidget *currentBestSuitParent();

    void setDefaultAction(DefaultAction defaultAction);
    void setAutomaticAnswer(const QString &identifier, QMessageBox::StandardButton answer);

    static QMessageBox::StandardButton critical(QWidget *parent, const QString &identifier,
        const QString &title, const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton button = QMessageBox::NoButton);

    static QMessageBox::StandardButton information(QWidget *parent, const QString &identifier,
        const QString &title, const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton button=QMessageBox::NoButton);

    static QMessageBox::StandardButton question(QWidget *parent, const QString &identifier,
        const QString &title, const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No,
        QMessageBox::StandardButton button = QMessageBox::NoButton);

    static QMessageBox::StandardButton warning(QWidget *parent, const QString &identifier,
        const QString &title, const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton button = QMessageBox::NoButton);

    Q_INVOKABLE int critical(const QString &identifier, const QString &title, const QString &text,
        int buttons = QMessageBox::Ok, int button = QMessageBox::NoButton);

    Q_INVOKABLE int information(const QString &identifier, const QString &title, const QString &text,
        int buttons = QMessageBox::Ok, int button = QMessageBox::NoButton);

    Q_INVOKABLE int question(const QString &identifier, const QString &title, const QString &text,
        int buttons = QMessageBox::Yes | QMessageBox::No, int button = QMessageBox::NoButton);

    Q_INVOKABLE int warning(const QString &identifier, const QString &title, const QString &text,
        int buttons = QMessageBox::Ok, int button = QMessageBox::NoButton);

    static QList<QMessageBox::Button> orderedButtons();

private Q_SLOTS:
    //this removes the slot from the script area
    virtual void deleteLater() {
        QObject::deleteLater();
    }

private:
    explicit MessageBoxHandler(QObject *parent);

    QMessageBox::StandardButton autoReply(QMessageBox::StandardButtons buttons) const;
    QMessageBox::StandardButton showMessageBox(MessageType messageType, QWidget *parent,
        const QString &identifier, const QString &title, const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        const QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);
    bool askAnswerFromUser(QMessageBox::StandardButton &selectedButton,
                           QMessageBox::StandardButtons &availableButtons) const;
    QString availableAnswerOptions(const QFlags<QMessageBox::StandardButton> &flags) const;


private:
    static MessageBoxHandler *m_instance;

    DefaultAction m_defaultAction;
    QList<QMessageBox::Button> m_buttonOrder;
    QHash<QString, QMessageBox::StandardButton> m_automaticAnswers;
};

}
Q_DECLARE_METATYPE(QMessageBox::StandardButton)
Q_DECLARE_METATYPE(QMessageBox::StandardButtons)

#endif // QINSTALLER_MESSAGEBOXHANDLER_H
