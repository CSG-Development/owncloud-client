#include "simpleresolveurljobfactory.h"

#include <QApplication>
#include <QNetworkReply>
#include <QLoggingCategory>

namespace {

Q_LOGGING_CATEGORY(lcSimpleResolveUrl, "libsync.simpleresolveurl")

// used to signalize that the request was aborted intentionally by the sslErrorHandler
const char abortedBySslErrorHandlerC[] = "aborted-by-ssl-error-handler";

QUrl concatUrlPath(const QUrl &url, const QString &concatPath, const QUrlQuery &queryItems = {})
{
    QString path = url.path();
    if (!concatPath.isEmpty()) {
        // avoid '//'
        if (path.endsWith(QLatin1Char('/')) && concatPath.startsWith(QLatin1Char('/'))) {
            path.chop(1);
        } // avoid missing '/'
        else if (!path.endsWith(QLatin1Char('/')) && !concatPath.startsWith(QLatin1Char('/'))) {
            path += QLatin1Char('/');
        }
        path += concatPath; // put the complete path together
    }
    Q_ASSERT(!path.contains(QStringLiteral("//")));
    Q_ASSERT(url.query().isEmpty());
    QUrl tmpUrl = url;
    tmpUrl.setPath(path);
    tmpUrl.setQuery(queryItems);
    return tmpUrl;
}

}

namespace APP {

SimpleResolveUrlJobFactory::SimpleResolveUrlJobFactory(QNetworkAccessManager *nam)
    : AbstractCoreJobFactory(nam)
{
}

CoreJob *SimpleResolveUrlJobFactory::startJob(const QUrl &url, QObject *parent)
{
    QNetworkRequest req(::concatUrlPath(url, QStringLiteral("status.php")));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    auto *job = new CoreJob(nam()->get(req), parent);

    auto makeFinishedHandler = [=](QNetworkReply *reply) {
        return [oldUrl = url, reply, job] {
            if (reply->error() != QNetworkReply::NoError) {
                if (reply->property(abortedBySslErrorHandlerC).toBool()) {
                    return;
                }

                qCCritical(lcSimpleResolveUrl) << QStringLiteral("Failed to resolve URL %1, error: %2").arg(oldUrl.toDisplayString(), reply->errorString());

                setJobError(job, QApplication::translate("SimpleResolveUrlJobFactory", "Could not detect compatible server at %1").arg(oldUrl.toDisplayString()));
                qCWarning(lcSimpleResolveUrl) << job->errorMessage();
                return;
            }

            const auto newUrl = reply->url().adjusted(QUrl::RemoveFilename);
            setJobResult(job, newUrl);
        };
    };

    QObject::connect(job->reply(), &QNetworkReply::finished, job, makeFinishedHandler(job->reply()));

    QObject::connect(job->reply(), &QNetworkReply::sslErrors, job, [req, job, makeFinishedHandler, nam = nam()](const QList<QSslError> &errors) mutable {
        job->reply()->setProperty(abortedBySslErrorHandlerC, true);
        job->reply()->abort();

        for (const auto &error : errors) {
            Q_EMIT job->caCertificateAccepted(error.certificate());
        }
        auto *reply = nam->get(req);
        QObject::connect(reply, &QNetworkReply::finished, job, makeFinishedHandler(reply));
    });

    makeRequest();

    return job;
}

} // namespace APP
