#pragma once

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

namespace securapp {
namespace handlers {

class BaseHandler : public proxygen::RequestHandler {
public:
    BaseHandler();
    virtual ~BaseHandler() = default;

    // RequestHandler implementation
    void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;
    void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
    void onEOM() noexcept override;
    void onUpgrade(proxygen::UpgradeProtocol proto) noexcept override;
    void requestComplete() noexcept override;
    void onError(proxygen::ProxygenError err) noexcept override;

protected:
    // Child classes implement this method to handle the request
    virtual void handleRequest() = 0;

    // Helper methods
    void sendErrorResponse(uint16_t statusCode, const std::string& errorMessage);
    void sendJsonResponse(uint16_t statusCode, const json& jsonBody);

    // Request data
    std::unique_ptr<proxygen::HTTPMessage> headers_;
    std::string body_;
    json jsonBody_;
    bool hasJsonBody_ = false;
};

} // namespace handlers
} // namespace securapp
