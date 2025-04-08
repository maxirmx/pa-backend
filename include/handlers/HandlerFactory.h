#pragma once

#include <proxygen/httpserver/RequestHandlerFactory.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace securapp {
namespace handlers {

class HandlerFactory : public proxygen::RequestHandlerFactory {
public:
    explicit HandlerFactory(const json& config);
    ~HandlerFactory() override = default;

    // Create a handler for a new request
    void onServerStart(folly::EventBase* evb) noexcept override;
    void onServerStop() noexcept override;
    proxygen::RequestHandler* onRequest(proxygen::RequestHandler*, proxygen::HTTPMessage* message) noexcept override;

private:
    // Configuration
    json config_;
    // Server event base
    folly::EventBase* evb_ = nullptr;
};

} // namespace handlers
} // namespace securapp
