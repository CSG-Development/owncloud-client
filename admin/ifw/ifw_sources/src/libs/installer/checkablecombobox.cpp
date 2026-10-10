/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "checkablecombobox.h"
#include "productkeycheck.h"
#include "messageboxhandler.h"

#include <QStandardItem>
#include <QApplication>
#include <QListView>

using namespace QInstaller;

CheckableComboBox::CheckableComboBox(const QString &placeholderText, QWidget *parent)
    : QComboBox(parent)
{
    //Workaround for showing the checkboxes in Linux
#ifndef Q_OS_WINDOWS
    setView(new QListView);
#endif
    setPlaceholderText(placeholderText);

    // Set combobox width, if not set the size is too wide
    const int iconWidth = iconSize().width();
    QFontMetrics fm = fontMetrics();
    int value = fm.horizontalAdvance(placeholderText) + iconWidth + 30; // add also margin
    setMaximumWidth(value);
}

void CheckableComboBox::addCheckableItem(const QString &text, const QString &tooltip, bool isChecked)
{
    QStandardItem *item = new QStandardItem(text);
    item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
    item->setToolTip(tooltip);

    // There is a bug which draws the checkbox on top of text in QWindows11 style and stylesheet style
    // Add a fake transparent icon to make space for the checkbox, see bug PYSIDE-2906
#ifdef Q_OS_WINDOWS
    if (QApplication::style()->name() == QLatin1String("windows11") || QApplication::style()->name().isEmpty()) {
        if (QApplication::isLeftToRight()) {
            QPixmap pixmap(32, 32);
            pixmap.fill(Qt::transparent);
            QIcon icon(pixmap);
            item->setIcon(icon);
        } else {
            item->setTextAlignment(Qt::AlignRight);
        }
    }
#endif

    if (isChecked)
        item->setData(Qt::Checked, Qt::CheckStateRole);
    else
        item->setData(Qt::Unchecked, Qt::CheckStateRole);
    qobject_cast<QStandardItemModel *>(model())->appendRow(item);
}

QStringList CheckableComboBox::checkedItems() const
{
    QStringList checked;
    for (int i = 0; i < count(); ++i) {
        QStandardItem *item = qobject_cast<QStandardItemModel *>(model())->item(i);
        if (item->checkState() == Qt::Checked)
            checked.append(item->text());
    }
    return checked;
}

QStringList CheckableComboBox::uncheckedItems() const
{
    QStringList unchecked;
    for (int i = 0; i < count(); ++i) {
        QStandardItem *item = qobject_cast<QStandardItemModel *>(model())->item(i);
        if (item->checkState() == Qt::Unchecked)
            unchecked.append(item->text());
    }
    return unchecked;
}

void CheckableComboBox::hidePopup()
{
    setCurrentIndex(-1);
    QComboBox::hidePopup();
}

void CheckableComboBox::showPopup()
{
    std::vector<int> arr;
    QFontMetrics fm = fontMetrics();
    for (int i = 0; i < count(); i++)
        arr.push_back(fm.horizontalAdvance(itemText(i)));

    int max_value = *std::max_element(arr.begin(), arr.end());
    max_value += 42; // add checkbox pixmap size plus margin
    view()->setMinimumWidth(max_value);
    QComboBox::showPopup();
}

void CheckableComboBox::updateCheckbox(int index)
{
    if (index != -1) {
        QStandardItemModel *comboModel=qobject_cast<QStandardItemModel*>(model());
        if (comboModel->item(index)->checkState() == Qt::Checked)
            comboModel->item(index)->setCheckState(Qt::Unchecked);
        else if (comboModel->item(index)->checkState() == Qt::Unchecked)
            comboModel->item(index)->setCheckState(Qt::Checked);
        emit currentIndexesChanged();
    }
}
