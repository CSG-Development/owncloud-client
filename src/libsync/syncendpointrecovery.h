#pragma once

#include "endpointrecoveryevent.h"
#include "personalcloudlib.h"

#include <QDateTime>
#include <QLoggingCategory>
#include <QNetworkReply>
#include <QUrl>

namespace APP {

Q_DECLARE_LOGGING_CATEGORY(lcSyncEndpointRecovery)

APPLICATIONSYNC_EXPORT EndpointRecoveryReason classifyEndpointRecoveryReason(QNetworkReply::NetworkError networkError, int httpStatusCode);
APPLICATIONSYNC_EXPORT EndpointRecoveryReason classifyEndpointRecoveryReason(QNetworkReply::NetworkError networkError, int httpStatusCode, bool timedOut);
APPLICATIONSYNC_EXPORT bool shouldScheduleEndpointRecovery(EndpointRecoveryReason reason);
APPLICATIONSYNC_EXPORT QString endpointRecoveryReasonString(EndpointRecoveryReason reason);
APPLICATIONSYNC_EXPORT EndpointRecoveryEvent makeEndpointRecoveryEvent(
    const QUuid &accountId,
    const std::optional<QUuid> &activePathId,
    const QUrl &baseUrl,
    EndpointRecoveryReason reason,
    QNetworkReply::NetworkError networkError,
    int httpStatusCode,
    const QDateTime &timestampUtc = {});

}
