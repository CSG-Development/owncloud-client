#include "Telemetry.h"

namespace APP {

Telemetry::Telemetry(std::unique_ptr<ProviderBase> provider)
    : provider_(std::move(provider))
{
}

Telemetry::~Telemetry()
{
    Finalize();
}

void Telemetry::Initialize()
{
    provider_->Initialize();
}

void Telemetry::Finalize()
{
    provider_->Finalize();
}

void Telemetry::sendEvent(const std::string &requestType, std::any &event)
{
    provider_->sendEvent(requestType, event);
}


} // namespace APP
