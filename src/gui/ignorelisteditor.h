/*
 * Copyright (C) by Daniel Molkentin <danimo@owncloud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#ifndef IGNORELISTEDITOR_H
#define IGNORELISTEDITOR_H

#include "gui/customui/stylehelper.h"

#include <QDialog>
#include <QFrame>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace APP {

namespace Ui {
    class IgnoreListEditor;
}

class InpDlg : public QDialog
{
    Q_OBJECT
public:
    InpDlg(QWidget* parent = nullptr)
        : QDialog(parent)
    {
        resize(440, 110);
        verticalLayout = new QVBoxLayout(this);
        frame = new QFrame(this);
        formLayout = new QFormLayout(frame);
        label = new QLabel(frame);
        formLayout->setWidget(0, QFormLayout::LabelRole, label);
        lineEdit = new QLineEdit(frame);
        formLayout->setWidget(0, QFormLayout::FieldRole, lineEdit);
        verticalLayout->addWidget(frame);
        frameButtons = new QFrame(this);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(frameButtons->sizePolicy().hasHeightForWidth());
        frameButtons->setSizePolicy(sizePolicy);
        horizontalLayout = new QHBoxLayout(frameButtons);
        horizontalSpacer = new QSpacerItem(236, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
        horizontalLayout->addItem(horizontalSpacer);
        btnOk = new QPushButton(frameButtons);
        horizontalLayout->addWidget(btnOk);
        btnCancel = new QPushButton(frameButtons);
        horizontalLayout->addWidget(btnCancel);
        verticalLayout->addWidget(frameButtons);
        QObject::connect(btnOk, &QPushButton::clicked, this, &InpDlg::accept);
        QObject::connect(btnCancel, &QPushButton::clicked, this, &InpDlg::reject);
        StyleHelper::applyPushButtonStyle(this);
        setWindowTitle(tr("Add Ignore Pattern"));
        label->setText(tr("Add a new ignore pattern:"));
        btnOk->setText(tr("OK"));
        btnCancel->setText(tr("Cancel"));
    }
    QString textValue() const {return lineEdit->text();}
private:
    QVBoxLayout *verticalLayout = nullptr;
    QFrame *frame = nullptr;
    QFormLayout *formLayout = nullptr;
    QLabel *label = nullptr;
    QLineEdit *lineEdit = nullptr;
    QFrame *frameButtons = nullptr;
    QHBoxLayout *horizontalLayout = nullptr;
    QSpacerItem *horizontalSpacer = nullptr;
    QPushButton *btnOk = nullptr;
    QPushButton *btnCancel = nullptr;
};


/**
 * @brief The IgnoreListEditor class
 * @ingroup gui
 */
class IgnoreListEditor : public QDialog
{
    Q_OBJECT

public:
    explicit IgnoreListEditor(QWidget *parent = nullptr);
    ~IgnoreListEditor() override;

    void updateTheme(bool isDark);

private slots:
    void slotItemSelectionChanged();
    void slotRemoveCurrentItem();
    void slotUpdateLocalIgnoreList();
    void slotAddPattern();

private:
    void readIgnoreFile(const QString &file, bool global);
    int addPattern(const QString &pattern, bool deletable, bool readOnly, bool global,
        const QStringList &skippedLines = QStringList());
    QString readOnlyTooltip;
    Ui::IgnoreListEditor *ui;
};

} // namespace APP

#endif // IGNORELISTEDITOR_H
