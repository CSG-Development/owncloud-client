#pragma once

#include "ProviderBase.h"

#include <memory>

namespace APP {

class Telemetry
{
public:
    explicit Telemetry(std::unique_ptr<ProviderBase> provider);
    ~Telemetry();

    void Initialize();
    void Finalize();

    void sendEvent(const std::string& requestType, std::any& event);

private:
    std::unique_ptr<ProviderBase> provider_;
};

} // namespace APP
