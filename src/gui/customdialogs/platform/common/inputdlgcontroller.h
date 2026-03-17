#pragma once

#include <QProperty>
#include <QObject>

class InputDlgController: public QObject
{
    Q_OBJECT

public:
    explicit InputDlgController(QObject* parent = nullptr);

    QProperty<QString> headerText;
    QProperty<QString> promptText;
    QProperty<QString> inputText;
    QProperty<QString> acceptButtonText {tr("OK")};
    QProperty<QString> rejectButtonText {tr("Cancel")};
    QProperty<bool> darkTheme {false};
};
