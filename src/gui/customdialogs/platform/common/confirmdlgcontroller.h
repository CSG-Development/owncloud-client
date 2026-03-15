#pragma once

#include <QProperty>
#include <QObject>

class ConfirmDlgController: public QObject
{
    Q_OBJECT

public:
    explicit ConfirmDlgController(QObject* parent = nullptr);

    QProperty<QString> headerText;
    QProperty<QString> messageText;
    QProperty<QString> acceptButtonText {tr("Yes")};
    QProperty<QString> rejectButtonText {tr("No")};
    QProperty<bool> warningIconVisible {false};
    QProperty<bool> isWide {false};
    QProperty<bool> darkTheme {false};
    QProperty<bool> singleButton {false};
};
