/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#ifndef ABOUTAPPLICATIONDIALOG_H
#define ABOUTAPPLICATIONDIALOG_H

#include <QDialog>

namespace QInstaller {
    class PackageManagerCore;
}

class AboutApplicationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutApplicationDialog(QInstaller::PackageManagerCore *core,
                                    QWidget *parent = nullptr);
    ~AboutApplicationDialog() = default;
};

#endif // ABOUTAPPLICATIONDIALOG_H
