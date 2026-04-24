#include "syncendpointrecovery.h"

namespace APP {

Q_LOGGING_CATEGORY(lcSyncEndpointRecovery, "sync.endpointrecovery", QtInfoMsg)

EndpointRecoveryReason classifyEndpointRecoveryReason(QNetworkReply::NetworkError networkError, int httpStatusCode)
{
    return classifyEndpointRecoveryReason(networkError, httpStatusCode, false);
}

EndpointRecoveryReason classifyEndpointRecoveryReason(QNetworkReply::NetworkError networkError, int httpStatusCode, bool timedOut)
{
    if (timedOut) {
        return EndpointRecoveryReason::Timeout;
    }

    switch (networkError) {
    case QNetworkReply::NoError:
        break;
    case QNetworkReply::HostNotFoundError:
        return EndpointRecoveryReason::HostResolutionFailed;
    case QNetworkReply::ConnectionRefusedError:
        return EndpointRecoveryReason::ConnectionRefused;
    case QNetworkReply::TimeoutError:
        return EndpointRecoveryReason::Timeout;
    case QNetworkReply::TemporaryNetworkFailureError:
        return EndpointRecoveryReason::TemporaryNetworkFailure;
    case QNetworkReply::RemoteHostClosedError:
        return EndpointRecoveryReason::RemoteHostClosed;
    case QNetworkReply::SslHandshakeFailedError:
        return EndpointRecoveryReason::TlsHandshakeFailed;
    case QNetworkReply::AuthenticationRequiredError:
        return EndpointRecoveryReason::Unauthorized;
    case QNetworkReply::OperationCanceledError:
        return EndpointRecoveryReason::NonRecoverableSyncError;
    default:
        if (networkError > QNetworkReply::NoError && networkError <= QNetworkReply::UnknownProxyError) {
            return EndpointRecoveryReason::TransportUnreachable;
        }
        break;
    }

    switch (httpStatusCode) {
    case 401:
    case 403:
        return EndpointRecoveryReason::Unauthorized;
    case 502:
    case 503:
    case 504:
        return EndpointRecoveryReason::ServerUnavailable;
    default:
        break;
    }

    return EndpointRecoveryReason::NonRecoverableSyncError;
}

bool shouldScheduleEndpointRecovery(EndpointRecoveryReason reason)
{
    switch (reason) {
    case EndpointRecoveryReason::TransportUnreachable:
    case EndpointRecoveryReason::HostResolutionFailed:
    case EndpointRecoveryReason::ConnectionRefused:
    case EndpointRecoveryReason::Timeout:
    case EndpointRecoveryReason::TemporaryNetworkFailure:
    case EndpointRecoveryReason::RemoteHostClosed:
    case EndpointRecoveryReason::TlsHandshakeFailed:
    case EndpointRecoveryReason::ServerUnavailable:
        return true;
    case EndpointRecoveryReason::Unauthorized:
    case EndpointRecoveryReason::PathSemanticallyInvalid:
    case EndpointRecoveryReason::NonRecoverableSyncError:
        return false;
    }

    return false;
}

QString endpointRecoveryReasonString(EndpointRecoveryReason reason)
{
    switch (reason) {
    case EndpointRecoveryReason::TransportUnreachable:
        return QStringLiteral("transport_unreachable");
    case EndpointRecoveryReason::HostResolutionFailed:
        return QStringLiteral("host_resolution_failed");
    case EndpointRecoveryReason::ConnectionRefused:
        return QStringLiteral("connection_refused");
    case EndpointRecoveryReason::Timeout:
        return QStringLiteral("timeout");
    case EndpointRecoveryReason::TemporaryNetworkFailure:
        return QStringLiteral("temporary_network_failure");
    case EndpointRecoveryReason::RemoteHostClosed:
        return QStringLiteral("remote_host_closed");
    case EndpointRecoveryReason::TlsHandshakeFailed:
        return QStringLiteral("tls_handshake_failed");
    case EndpointRecoveryReason::ServerUnavailable:
        return QStringLiteral("server_unavailable");
    case EndpointRecoveryReason::Unauthorized:
        return QStringLiteral("unauthorized");
    case EndpointRecoveryReason::PathSemanticallyInvalid:
        return QStringLiteral("path_semantically_invalid");
    case EndpointRecoveryReason::NonRecoverableSyncError:
        return QStringLiteral("non_recoverable_sync_error");
    }

    return QStringLiteral("unknown");
}

EndpointRecoveryEvent makeEndpointRecoveryEvent(
    const QUuid &accountId,
    const std::optional<QUuid> &activePathId,
    const QUrl &baseUrl,
    EndpointRecoveryReason reason,
    QNetworkReply::NetworkError networkError,
    int httpStatusCode,
    const QDateTime &timestampUtc)
{
    EndpointRecoveryEvent event;
    event.accountId = accountId;
    event.activePathId = activePathId;
    event.baseUrl = baseUrl.toString();
    event.reason = reason;
    event.networkError = static_cast<int>(networkError);
    event.httpStatus = httpStatusCode;
    event.timestampUtc = timestampUtc.isValid() ? timestampUtc : QDateTime::currentDateTimeUtc();
    return event;
}

}
