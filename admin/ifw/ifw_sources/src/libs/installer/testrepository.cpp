/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#include "testrepository.h"

#include "packagemanagercore.h"
#include "packagemanagerproxyfactory.h"
#include "proxycredentialsdialog.h"
#include "serverauthenticationdialog.h"

#include <QFile>
#include <QRandomGenerator>

namespace QInstaller {

/*!
    \inmodule QtInstallerFramework
    \class QInstaller::TestRepository
    \internal
*/

TestRepository::TestRepository(PackageManagerCore *parent)
    : Job(parent)
    , m_core(parent)
{
    setAutoDelete(false);
    setCapabilities(Cancelable);

    connect(&m_timer, &QTimer::timeout, this, &TestRepository::onTimeout);
    connect(&m_xmlTask, &QFutureWatcherBase::finished, this, &TestRepository::downloadCompleted);
}

TestRepository::~TestRepository()
{
    reset();
}

Repository TestRepository::repository() const
{
    return m_repository;
}

void TestRepository::setRepository(const Repository &repository)
{
    reset();
    m_repository = repository;
}

void TestRepository::doStart()
{
    reset();
    if (!m_core) {
        emitFinishedWithError(Job::Canceled, tr("Missing package manager core engine."));
        return; // We can't do anything here without core, so avoid tons of !m_core checks.
    }

    const QUrl url = m_repository.url();
    if (url.isEmpty()) {
        emitFinishedWithError(QInstaller::InvalidUrl, tr("Empty repository URL."));
        return;
    }

    QAuthenticator auth;
    auth.setUser(m_repository.username());
    auth.setPassword(m_repository.password());

    FileTaskItem item(m_repository.url().toString() + QLatin1String("/Updates.xml?") +
        QString::number(QRandomGenerator::global()->generate()));
    item.insert(TaskRole::Authenticator, QVariant::fromValue(auth));

    m_timer.start(10000);
    DownloadFileTask *const xmlTask = new DownloadFileTask(item);
    if (m_core)
        xmlTask->setProxyFactory(m_core->proxyFactory());
    m_xmlTask.setFuture(QtConcurrent::run(&DownloadFileTask::doTask, xmlTask));
}

void TestRepository::doCancel()
{
    reset();
    emitFinishedWithError(Job::Canceled, tr("Download canceled."));
}

void TestRepository::onTimeout()
{
    reset();
    emitFinishedWithError(Job::Canceled, tr("Timeout while testing repository \"%1\".")
        .arg(m_repository.displayname()));
}

void TestRepository::downloadCompleted()
{
    if (error() != Job::NoError)
        return;

    try {
        m_xmlTask.waitForFinished();

        m_timer.stop();
        QFile file(m_xmlTask.future().results().value(0).target());
        if (file.open(QIODevice::ReadOnly)) {
            QDomDocument doc;
            QDomDocument::ParseResult result = doc.setContent(&file);
            if (!result) {
                emitFinishedWithError(QInstaller::InvalidUpdatesXml,
                    tr("Cannot parse Updates.xml: %1").arg(result.errorMessage));
            } else {
                emitFinishedWithError(Job::NoError, QString(/*Success*/)); // OPK
            }
        } else {
            emitFinishedWithError(QInstaller::DownloadError,
                tr("Cannot open Updates.xml for reading: %1").arg(file.errorString()));
        }
    } catch (const AuthenticationRequiredException &e) {
        m_timer.stop();
        if (e.type() == AuthenticationRequiredException::Type::Server) {
            ServerAuthenticationDialog dlg(e.message(), e.taskItem());
            if (dlg.exec() == QDialog::Accepted) {
                m_repository.setUsername(dlg.user());
                m_repository.setPassword(dlg.password());
                QMetaObject::invokeMethod(this, "doStart", Qt::QueuedConnection);
            } else {
                QMetaObject::invokeMethod(this, "doCancel", Qt::QueuedConnection);
            }
            return;
        } else if (e.type() == AuthenticationRequiredException::Type::Proxy) {
            const QNetworkProxy proxy = e.proxy();
            ProxyCredentialsDialog proxyCredentials(proxy);
            if (proxyCredentials.exec() == QDialog::Accepted) {
                PackageManagerProxyFactory *factory = m_core->proxyFactory();
                factory->setProxyCredentials(proxy, proxyCredentials.userName(),
                    proxyCredentials.password());
                m_core->setProxyFactory(factory);
            }
            QMetaObject::invokeMethod(this, "doStart", Qt::QueuedConnection);
            return;
        } else {
            emitFinishedWithError(QInstaller::DownloadError, tr("Authentication failed."));
        }
    } catch (const TaskException &e) {
        m_timer.stop();
        emitFinishedWithError(QInstaller::DownloadError, e.message());
    } catch (const QUnhandledException &e) {
        m_timer.stop();
        emitFinishedWithError(QInstaller::DownloadError, QLatin1String(e.what()));
    } catch (...) {
        m_timer.stop();
        emitFinishedWithError(QInstaller::DownloadError,
            tr("Unknown error while testing repository \"%1\".").arg(m_repository.displayname()));
    }
}


// -- private

void TestRepository::reset()
{
    m_timer.stop();
    setError(NoError);
    setErrorString(QString());

    try {
        if (m_xmlTask.isRunning())
            m_xmlTask.cancel();
    } catch (...) {}
}

} // namespace QInstaller
