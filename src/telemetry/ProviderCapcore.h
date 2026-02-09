#pragma once

#include "TelemetryClient.h"
#include "ProviderBase.h"

#include <any>

namespace APP {

class ProviderCapcore: public ProviderBase
{
public:
    ProviderCapcore();
    ~ProviderCapcore() override;

    void Initialize() override;
    void Finalize() override;

    void sendEvent(const std::string& requestType, std::any& event) override;

    ::Telemetry::TelemetryClient* client_ = nullptr;
};


} // namespace APP
