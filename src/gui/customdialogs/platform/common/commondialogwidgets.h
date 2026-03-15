#pragma once

class QLabel;
class QPushButton;
class QToolButton;
class QWidget;
class QFrame;
class QLineEdit;

struct CommonDialogWidgets
{
    QLabel* lblHeader = nullptr;
    QLabel* lblText = nullptr;
    QPushButton* btnAccept = nullptr;
    QPushButton* btnReject = nullptr;
    QToolButton* btnIcon = nullptr;
    QToolButton* btnIconWarn = nullptr;
    QFrame* frameIcon = nullptr;
    QFrame* frameAcceptBtn = nullptr;
    QFrame* frameRejectBtn = nullptr;
    QWidget* frame = nullptr;
    QWidget* frameHeader = nullptr;
    QLineEdit* edText = nullptr;
};
