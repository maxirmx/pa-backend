#include "handlers/HandlerFactory.h"
#include "handlers/HealthCheckHandler.h"
#include "handlers/NotFoundHandler.h"
#include "handlers/ApiHandler.h"
#include <glog/logging.h>
#include <folly/Uri.h>

namespace securapp {
namespace handlers {

HandlerFactory::HandlerFactory(const json& config) : config_(config) {
    LOG(INFO) << "Handler factory initialized";
}

void HandlerFactory::onServerStart(folly::EventBase* evb) noexcept {
    LOG(INFO) << "Server started";
    evb_ = evb;
}

void HandlerFactory::onServerStop() noexcept {
    LOG(INFO) << "Server stopped";
}

proxygen::RequestHandler* HandlerFactory::onRequest(
    proxygen::RequestHandler* /* handler */,
    proxygen::HTTPMessage* message) noexcept {

    try {
        // Get the path from the request
        folly::Uri uri(message->getURL());
        std::string path = uri.path();

        LOG(INFO) << "Request received: " << message->getMethodString() << " " << path;

        // Route the request to the appropriate handler
        if (path == "/health" || path == "/health/") {
            return new HealthCheckHandler();
        } else if (path.find("/api/") == 0) {
            return new ApiHandler(config_);
        } else {
            return new NotFoundHandler();
        }
    } catch (const std::exception& e) {
        LOG(ERROR) << "Error routing request: " << e.what();
        return new NotFoundHandler();
    }
}

} // namespace handlers
} // namespace securapp
