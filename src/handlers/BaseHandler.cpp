#include "handlers/BaseHandler.h"
#include <glog/logging.h>
#include <folly/io/IOBuf.h>
#include <folly/Conv.h>

namespace securapp {
namespace handlers {

BaseHandler::BaseHandler() = default;

void BaseHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {
    headers_ = std::move(headers);
}

void BaseHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
    if (body) {
        std::string bodyPart = body->moveToFbString().toStdString();
        body_.append(bodyPart);
    }
}

void BaseHandler::onEOM() noexcept {
    try {
        // If there's a body and Content-Type is application/json, parse it
        if (!body_.empty() &&
            headers_->getHeaders().getSingleOrEmpty("Content-Type").find("application/json") != std::string::npos) {

            jsonBody_ = json::parse(body_);
            hasJsonBody_ = true;
        }
    } catch (const json::exception& e) {
        LOG(ERROR) << "JSON parsing error: " << e.what();
        sendErrorResponse(400, "Invalid JSON in request body");
        return;
    } catch (const std::exception& e) {
        LOG(ERROR) << "Error processing request body: " << e.what();
        sendErrorResponse(500, "Internal server error");
        return;
    }

    // Process the request
    handleRequest();
}

void BaseHandler::onUpgrade(proxygen::UpgradeProtocol proto) noexcept {
    // This server doesn't support upgrades
    LOG(WARNING) << "Upgrade protocol not supported: " << static_cast<int>(proto);
}

void BaseHandler::requestComplete() noexcept {
    // Request is complete, delete this handler
    delete this;
}

void BaseHandler::onError(proxygen::ProxygenError err) noexcept {
    LOG(ERROR) << "Request handler error: " << proxygen::getErrorString(err);
    // Request is complete with error, delete this handler
    delete this;
}

void BaseHandler::sendErrorResponse(uint16_t statusCode, const std::string& errorMessage) {
    json errorJson = {
        {"status", "error"},
        {"message", errorMessage}
    };

    proxygen::ResponseBuilder(downstream_)
        .status(statusCode, errorMessage)
        .header("Content-Type", "application/json")
        .body(errorJson.dump(2))  // indent with 2 spaces
        .sendWithEOM();
}

void BaseHandler::sendJsonResponse(uint16_t statusCode, const json& jsonBody) {
    proxygen::ResponseBuilder(downstream_)
        .status(statusCode, "OK")
        .header("Content-Type", "application/json")
        .body(jsonBody.dump(2))  // indent with 2 spaces
        .sendWithEOM();
}

} // namespace handlers
} // namespace securapp
