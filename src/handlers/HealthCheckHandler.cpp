#include "handlers/HealthCheckHandler.h"
#include "db/DatabaseManager.h"
#include <glog/logging.h>

namespace securapp {
namespace handlers {

void HealthCheckHandler::handleRequest() {
    // Check database connectivity
    bool dbConnected = db::DatabaseManager::getInstance().isConnected();

    json healthJson = {
        {"status", "ok"},
        {"timestamp", std::time(nullptr)},
        {"components", {
            {"database", {
                {"status", dbConnected ? "up" : "down"}
            }},
            {"server", {
                {"status", "up"}
            }}
        }}
    };

    sendJsonResponse(200, healthJson);
    LOG(INFO) << "Health check completed: " << (dbConnected ? "all systems up" : "database is down");
}

} // namespace handlers
} // namespace securapp
