#include "handlers/ApiHandler.h"
#include "db/DatabaseManager.h"
#include <glog/logging.h>
#include <folly/dynamic.h>
#include <folly/Uri.h>
#include <algorithm>
#include <vector>

namespace securapp {
namespace handlers {

ApiHandler::ApiHandler(const json& config) : config_(config) {}

void ApiHandler::handleRequest() {
    try {
        // Get the path from the request
        std::string path = headers_->getPath();
        std::string method = headers_->getMethodString();

        LOG(INFO) << "API request: " << method << " " << path;

        // Extract endpoint from path (format: /api/{endpoint}/{parameters})
        std::vector<std::string> pathParts;
        std::string part;
        std::istringstream pathStream(path);

        // Skip the first part (empty due to leading slash)
        std::getline(pathStream, part, '/');

        while (std::getline(pathStream, part, '/')) {
            if (!part.empty()) {
                pathParts.push_back(part);
            }
        }

        // We expect at least "api" and an endpoint
        if (pathParts.size() < 2 || pathParts[0] != "api") {
            sendErrorResponse(404, "API endpoint not found");
            return;
        }

        std::string endpoint = pathParts[1];

        // Route to the appropriate endpoint handler
        if (endpoint == "users") {
            handleUsersEndpoint();
        } else if (endpoint == "auth") {
            handleAuthEndpoint();
        } else {
            sendErrorResponse(404, "Unknown API endpoint: " + endpoint);
        }
    } catch (const std::exception& e) {
        LOG(ERROR) << "API error: " << e.what();
        sendErrorResponse(500, "Internal server error");
    }
}

void ApiHandler::handleUsersEndpoint() {
    std::string method = headers_->getMethodString();

    if (method == "GET") {
        // In a real application, we would use the database here
        // For now, return mock data
        json response = {
            {"users", json::array({
                {
                    {"id", "1"},
                    {"username", "demo_user"},
                    {"email", "demo@example.com"},
                    {"created_at", "2023-01-01T00:00:00Z"}
                },
                {
                    {"id", "2"},
                    {"username", "admin_user"},
                    {"email", "admin@example.com"},
                    {"created_at", "2023-01-02T00:00:00Z"}
                }
            })}
        };

        sendJsonResponse(200, response);
    } else if (method == "POST" && hasJsonBody_) {
        // In a real application, we would validate and store the user data
        if (!jsonBody_.contains("username") ||
            !jsonBody_.contains("email") ||
            !jsonBody_.contains("password")) {

            sendErrorResponse(400, "Missing required fields (username, email, password)");
            return;
        }

        // Mock successful user creation
        json response = {
            {"status", "success"},
            {"message", "User created successfully"},
            {"user", {
                {"id", "3"},
                {"username", jsonBody_["username"]},
                {"email", jsonBody_["email"]},
                {"created_at", "2023-04-08T00:00:00Z"}
            }}
        };

        sendJsonResponse(201, response);
    } else {
        sendErrorResponse(405, "Method not allowed");
    }
}

void ApiHandler::handleAuthEndpoint() {
    std::string method = headers_->getMethodString();

    if (method == "POST" && hasJsonBody_) {
        // In a real application, we would verify credentials
        if (!jsonBody_.contains("username") || !jsonBody_.contains("password")) {
            sendErrorResponse(400, "Missing credentials");
            return;
        }

        // Mock authentication success
        json response = {
            {"status", "success"},
            {"token", "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkRlbW8gVXNlciIsImlhdCI6MTUxNjIzOTAyMn0.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c"},
            {"expires_in", 3600}
        };

        sendJsonResponse(200, response);
    } else {
        sendErrorResponse(405, "Method not allowed");
    }
}

} // namespace handlers
} // namespace securapp
