#pragma once

#include <any>
#include <string>

namespace APP {

class ProviderBase
{
public:
    ProviderBase() = default;
    virtual ~ProviderBase() = default;

    virtual void Initialize() = 0;
    virtual void Finalize() = 0;

    virtual void sendEvent(const std::string& requestType, std::any& event) = 0;

};

} // namespace APP
