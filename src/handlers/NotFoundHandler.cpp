#include "handlers/NotFoundHandler.h"
#include <glog/logging.h>

namespace securapp {
namespace handlers {

void NotFoundHandler::handleRequest() {
    LOG(INFO) << "Resource not found: " << headers_->getPath();

    sendErrorResponse(404, "Resource not found");
}

} // namespace handlers
} // namespace securapp
