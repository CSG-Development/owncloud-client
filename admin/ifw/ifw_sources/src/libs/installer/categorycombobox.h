/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef CATEGORYCOMBOBOX_H
#define CATEGORYCOMBOBOX_H

#include "checkablecombobox.h"

namespace QInstaller {

class CategoryComboBox : public CheckableComboBox
{
    Q_OBJECT
public:
    CategoryComboBox(const QString &placeholderText, QWidget *parent = nullptr);

public Q_SLOTS:
    virtual void updateCheckbox(int index) override;
};

}   // namespace QInstaller

#endif // CATEGORYCOMBOBOX_H
