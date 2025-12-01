#pragma once

#include "abstractcorejob.h"

namespace CUR {

class CURATORSYNC_EXPORT SimpleResolveUrlJobFactory : public AbstractCoreJobFactory
{
public:
    explicit SimpleResolveUrlJobFactory(QNetworkAccessManager *nam);

    CoreJob *startJob(const QUrl &url, QObject *parent) override;
};

} // namespace CUR
