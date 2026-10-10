/* Copyright C 2013 Klaralvdalens Datakonsult AB KDAB
 * Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/

#include "selfrestarter.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QProcess>

class SelfRestarter::Private
{
public:
    Private(int argc, char *argv[])
        : executable(QString::fromLocal8Bit(argv[0]))
        , restartOnQuit(false)
        , workingPath(QDir::currentPath())
    {

        for (int i = 1; i < argc; ++i)
            args << QString::fromLocal8Bit(argv[i]);
    }

    Private()
        : executable(qApp->applicationFilePath())
        , args(qApp->arguments().mid(1))
        , restartOnQuit(false)
        , workingPath(QDir::currentPath())
    {
    }

    ~Private()
    {
        if (restartOnQuit)
            QProcess::startDetached(executable, args, workingPath);
    }

    QString executable;
    QStringList args;
    bool restartOnQuit;
    QString workingPath;
    static SelfRestarter *instance;
};

SelfRestarter *SelfRestarter::Private::instance = 0;

SelfRestarter::SelfRestarter(int argc, char *argv[])
    : d(new Private(argc, argv))
{
    Q_ASSERT_X(!Private::instance, Q_FUNC_INFO, "Cannot create more than one SelfRestarter instance");
    Private::instance = this;
}

SelfRestarter::~SelfRestarter()
{
    Q_ASSERT_X(Private::instance == this, Q_FUNC_INFO, "Cannot create more than one SelfRestarter instance");
    delete d;
    Private::instance = 0;
}

void SelfRestarter::setRestartOnQuit(bool restart)
{
    Q_ASSERT_X(Private::instance, Q_FUNC_INFO, "SelfRestarter instance must be created in main()");
    if (Private::instance)
        Private::instance->d->restartOnQuit = restart;
}

bool SelfRestarter::restartOnQuit()
{
    return Private::instance ? Private::instance->d->restartOnQuit : false;
}
