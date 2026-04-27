#pragma once

#include "personalcloudlib.h"

#include <QDateTime>
#include <QMetaType>
#include <QString>
#include <QUuid>

#include <optional>

namespace APP {

enum class EndpointRecoveryReason
{
    TransportUnreachable,
    HostResolutionFailed,
    ConnectionRefused,
    Timeout,
    TemporaryNetworkFailure,
    RemoteHostClosed,
    TlsHandshakeFailed,
    ServerUnavailable,
    Unauthorized,
    PathSemanticallyInvalid,
    NonRecoverableSyncError
};

struct APPLICATIONSYNC_EXPORT EndpointRecoveryEvent
{
    // This event describes endpoint-level recovery input and must not be
    // treated as a generic sync error.
    QUuid accountId;
    std::optional<QUuid> activePathId;
    QString baseUrl;
    EndpointRecoveryReason reason = EndpointRecoveryReason::NonRecoverableSyncError;
    int networkError = 0;
    int httpStatus = 0;
    QDateTime timestampUtc;

    bool isRecoverableTransportFailure() const;
    bool requiresReauthentication() const;
    bool isNonRecoverableSyncError() const;
};

inline bool EndpointRecoveryEvent::isRecoverableTransportFailure() const
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
    case EndpointRecoveryReason::PathSemanticallyInvalid:
        return true;
    case EndpointRecoveryReason::Unauthorized:
    case EndpointRecoveryReason::NonRecoverableSyncError:
        return false;
    }

    return false;
}

inline bool EndpointRecoveryEvent::requiresReauthentication() const
{
    return reason == EndpointRecoveryReason::Unauthorized;
}

inline bool EndpointRecoveryEvent::isNonRecoverableSyncError() const
{
    return reason == EndpointRecoveryReason::NonRecoverableSyncError;
}

}

Q_DECLARE_METATYPE(APP::EndpointRecoveryReason)
Q_DECLARE_METATYPE(APP::EndpointRecoveryEvent)
