#pragma once
#include "ProviderBase.h"

namespace APP {

class ProviderNull: public ProviderBase
{
public:
    ProviderNull() : ProviderBase() {}

    void Initialize() override {}
    void Finalize() override {}

    void sendEvent(const std::string& /*requestType*/, std::any& /*event*/) override {}
};

} // namespace APP
