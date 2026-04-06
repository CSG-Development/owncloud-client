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
#include "gui/customdialogs/dlgutils.h"
#include "gui/customui/focusproxy.h"
#include "gui/customdialogs/platform/common/windowdragger.h"

#include "theme.h"
#include "guiutility.h"

#include <QTabBar>

namespace {
const auto logo_icon = QStringLiteral(":/res/Files-logo.png");
const std::pair<QString,QString> widgetStyle = {
    QStringLiteral(":/res/about.qss"),
    QStringLiteral(":/res/about_mac.qss")
};
const std::pair<QString,QString> close_icon = {
    QStringLiteral(":/res/close_light.svg"),
    QStringLiteral(":/res/close_dark.svg"),
};

#ifdef Q_OS_MACOS
QMargins frameContentMargins{32, 0, 32, 0};
QMargins frameButtonsMargins{7, 7, 30, 7};
#else
QMargins frameContentMargins{24, 24, 24, 0};
#endif

}

namespace APP {

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    DlgUtils::setTransparent(this);
    DlgUtils::applyDropShadowDialog(ui->frame);
#ifdef Q_OS_WIN
    setStyleSheet(StyleHelper::loadFileToString(widgetStyle.first));
#else
    setStyleSheet(StyleHelper::loadFileToString(widgetStyle.second));
#endif

    ui->btnOk->setStyle(new FocusProxyStyle(ui->btnOk));

    StyleHelper::applyPushButtonsStyle(this);
#ifdef Q_OS_WINDOWS
    ui->btnIconTitle->setIcon(QIcon(logo_icon));
#else
    ui->btnIconTitle->setVisible(false);
    ui->btnHeadClose->setVisible(false);
    ui->lblHeaderTitle->setVisible(false);
#endif

    ui->btnIconTitle->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->btnIcon->setAttribute(Qt::WA_TransparentForMouseEvents);

    setWindowTitle(tr("About %1").arg(Theme::instance()->appNameGUI()));
    ui->aboutText->setText(Theme::instance()->about());
    connect(ui->aboutText, &QLabel::linkActivated, this, &AboutDialog::openBrowser);
    connect(ui->btnOk, &QPushButton::clicked, this, &AboutDialog::accept);
    connect(ui->btnHeadClose, &QPushButton::clicked, this, &AboutDialog::reject);

    connect(Theme::instance(), &Theme::themeChanged, this, &AboutDialog::updateTheme);
    updateTheme(APP::Theme::instance()->isDarkTheme());

    new WindowDragger(ui->frameTitle, this);

    QTimer::singleShot(0, [this, p = parent] {
        ui->frameContent->updateGeometry();
        ui->aboutText->updateGeometry();
        ui->frame->updateGeometry();
#ifdef Q_OS_MACOS
        ui->frameContent->setContentsMargins(frameContentMargins);
        ui->frameButtons->setContentsMargins(frameButtonsMaggins);
#else
        ui->frameContent->setContentsMargins(frameContentMargins);
#endif
        if (layout())
            layout()->activate();
        adjustSize();
        DlgUtils::centerDialog(p, this);
    });
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::updateTheme(bool isDark)
{
    DlgUtils::setTheme(this, isDark);
#ifdef Q_OS_WINDOWS
    ui->btnHeadClose->setIcon(isDark ? QIcon(close_icon.second) : QIcon(close_icon.first));
#endif
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
