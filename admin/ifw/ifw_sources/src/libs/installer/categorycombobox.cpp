/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "categorycombobox.h"
#include "checkablecombobox.h"
#include "productkeycheck.h"
#include "messageboxhandler.h"

#include <QStandardItem>
#include <QApplication>
#include <QListView>

using namespace QInstaller;

CategoryComboBox::CategoryComboBox(const QString &placeholderText, QWidget *parent)
    : CheckableComboBox(placeholderText, parent)
{
}

void CategoryComboBox::updateCheckbox(int index)
{
    if (index != -1) {
        QStandardItem *item = qobject_cast<QStandardItemModel *>(model())->item(index);
        QString acceptanceText = ProductKeyCheck::instance()->additionalAcceptanceForCategory(item);
        if (!acceptanceText.isEmpty() && item->checkState() == Qt::Unchecked) {
            const QMessageBox::StandardButton button =
                MessageBoxHandler::question(MessageBoxHandler::currentBestSuitParent(),
                QLatin1String("categoryAcceptance"), tr("Confirm"), acceptanceText,
                QMessageBox::Yes | QMessageBox::Cancel);
            if (button == QMessageBox::Cancel) {
                item->setCheckState(Qt::Unchecked);
                setCurrentIndex(-1);
                return;
            }
        }
        CheckableComboBox::updateCheckbox(index);
    }
}
