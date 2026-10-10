/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef CHECKABLECOMBOBOX_H
#define CHECKABLECOMBOBOX_H

#include <QComboBox>

namespace QInstaller {

class CheckableComboBox : public QComboBox
{
    Q_OBJECT
public:
    CheckableComboBox(const QString &placeholderText, QWidget *parent = nullptr);

public:
    void addCheckableItem(const QString &text, const QString &tooltip, bool isChecked);
    Q_INVOKABLE QStringList checkedItems() const;
    Q_INVOKABLE QStringList uncheckedItems() const;

Q_SIGNALS:
    void currentIndexesChanged();

public Q_SLOTS:
    virtual void updateCheckbox(int index);

protected:
    void hidePopup() override;
    void showPopup() override;
};

}   // namespace QInstaller

#endif // CHECKABLECOMBOBOX_H
