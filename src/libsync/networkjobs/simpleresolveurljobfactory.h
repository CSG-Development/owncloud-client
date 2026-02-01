#pragma once

#include "abstractcorejob.h"

namespace APP {

class APPLICATIONSYNC_EXPORT SimpleResolveUrlJobFactory : public AbstractCoreJobFactory
{
public:
    explicit SimpleResolveUrlJobFactory(QNetworkAccessManager *nam);

    CoreJob *startJob(const QUrl &url, QObject *parent) override;
};

} // namespace APP
