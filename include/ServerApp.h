#pragma once

#include <string>
#include <memory>
#include <proxygen/httpserver/HTTPServer.h>
#include <nlohmann/json.hpp>
#include <proxygen/httpserver/HTTPServerOptions.h>
#include <folly/io/async/EventBaseManager.h>

using json = nlohmann::json;

namespace securapp {

class ServerApp {
public:
    ServerApp();
    ~ServerApp();

    // Initialize the server with the given config file
    bool initialize(const std::string& configPath);

    // Start the server
    bool start();

    // Stop the server
    void stop();

    // Run the server until stopped
    void run();

private:
    // Load configuration from file
    bool loadConfig(const std::string& configPath);

    // Initialize database connection
    bool initDatabase();

    // Setup SSL/TLS if enabled
    bool setupSSL(proxygen::HTTPServerOptions& options);

    // Server configuration
    json config_;

    // Server instance
    std::unique_ptr<proxygen::HTTPServer> server_;

    // Event base manager
    folly::EventBaseManager eventBaseManager_;

    // Main event base
    folly::EventBase* mainEventBase_ = nullptr;

    // Flag to indicate if server is running
    bool running_ = false;
};

} // namespace securapp
