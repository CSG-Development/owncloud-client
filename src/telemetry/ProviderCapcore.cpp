#include "ProviderCapcore.h"
#include <QLoggingCategory>

namespace CUR {

Q_LOGGING_CATEGORY(lcCapcoreTelemetry, "telemetry.capcore")

ProviderCapcore::ProviderCapcore()
    : ProviderBase()
{
}

ProviderCapcore::~ProviderCapcore()
{
    client_ = nullptr;
}

void ProviderCapcore::Initialize()
{
    Json::Value config;
    config[Telemetry::ENABLED_TAG] = Telemetry::ENABLED_DEFAULT;
    Telemetry::TelemetryClient::init("client_id", "hw_id", "Curator Files", config);
}

void ProviderCapcore::Finalize()
{
    client_ = nullptr;
}

void ProviderCapcore::sendEvent(const string &requestType, std::any& event)
{
    if (event.has_value())
    {
        try {
            auto ev = std::any_cast<Telemetry::TelemetryEvent>(event);
            client_->sendEvent(requestType, ev);
        }
        catch (const std::bad_any_cast& e) {
            qCWarning(lcCapcoreTelemetry) << "Error event cast" << e.what();
        }
    }
}

} // namespace CUR
