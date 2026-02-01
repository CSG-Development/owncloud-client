/*
 * Copyright (C) by Hannah von Reth <hannah.vonreth@owncloud.com>
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
#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "customui/stylehelper.h"

#include "theme.h"
#include "guiutility.h"

#include <QTabBar>

namespace APP {

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    StyleHelper::applyPushButtonStyle(this);
    setWindowTitle(tr("About %1").arg(Theme::instance()->appNameGUI()));
    ui->aboutText->setText(Theme::instance()->about());
    connect(ui->aboutText, &QLabel::linkActivated, this, &AboutDialog::openBrowser);
    connect(ui->btnOk, &QPushButton::clicked, this, &AboutDialog::accept);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::openBrowser(const QString &s)
{
    Utility::openBrowser(QUrl(s), this);
}

void AboutDialog::openBrowserFromUrl(const QUrl &s)
{
    return openBrowser(s.toString());
}

}
