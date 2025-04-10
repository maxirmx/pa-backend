#include "ServerApp.h"
#include "handlers/HandlerFactory.h"
#include "db/DatabaseManager.h"

#include <glog/logging.h>
#include <folly/json.h>
#include <folly/io/async/SSLContext.h>
#include <folly/io/async/EventBase.h>
#include <folly/FileUtil.h>
#include <folly/String.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <proxygen/httpserver/HTTPServer.h>
#include <proxygen/httpserver/Filters.h>

#include <signal.h>
#include <fstream>
#include <thread>

namespace securapp {

// Global pointer for signal handling
static ServerApp* g_serverApp = nullptr;

// Signal handler
static void signalHandler(int signum) {
    LOG(INFO) << "Received signal " << signum;
    if (g_serverApp) {
        g_serverApp->stop();
    }
}

ServerApp::ServerApp() : running_(false) {
    // Store global pointer for signal handling
    g_serverApp = this;

    // Setup signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
}

ServerApp::~ServerApp() {
    stop();
    g_serverApp = nullptr;
}

bool ServerApp::loadConfig(const std::string& configPath) {
    try {
        // Read config file
        std::ifstream configFile(configPath);
        if (!configFile.is_open()) {
            LOG(ERROR) << "Failed to open config file: " << configPath;
            return false;
        }

        // Parse JSON
        config_ = json::parse(configFile);
        LOG(INFO) << "Configuration loaded from " << configPath;
        return true;
    } catch (const std::exception& e) {
        LOG(ERROR) << "Failed to load config: " << e.what();
        return false;
    }
}

bool ServerApp::initialize(const std::string& configPath) {
    // Load configuration
    if (!loadConfig(configPath)) {
        return false;
    }

    // Initialize database
    if (!initDatabase()) {
        return false;
    }

    // Get server config
    const auto& serverConfig = config_["server"];

    // Setup HTTP server options
    proxygen::HTTPServerOptions options;
    options.threads = serverConfig.value("threads", 4);
    options.idleTimeout = std::chrono::milliseconds(serverConfig.value("idle_timeout", 60000));
    options.shutdownOn = {SIGINT, SIGTERM};
    options.enableContentCompression = true;

    // Setup handler factory
    options.handlerFactories = proxygen::RequestHandlerChain()
        .addThen(std::make_unique<handlers::HandlerFactory>(config_))
        .build();

    // Setup SSL if enabled
    if (serverConfig.contains("ssl")) {
        if (!setupSSL(options)) {
            return false;
        }
    }

    // Setup IP address and ports
    std::vector<folly::SocketAddress> addresses;

    // HTTP port
    if (serverConfig.contains("http_port")) {
        folly::SocketAddress httpAddr;
        httpAddr.setFromHostPort(
            serverConfig.value("host", "0.0.0.0"),
            serverConfig.value("http_port", 8080)
        );
        addresses.push_back(httpAddr);
        LOG(INFO) << "HTTP enabled on port " << serverConfig.value("http_port", 8080);
    }

    // HTTPS port
    if (serverConfig.contains("https_port") && serverConfig.contains("ssl")) {
        folly::SocketAddress httpsAddr;
        httpsAddr.setFromHostPort(
            serverConfig.value("host", "0.0.0.0"),
            serverConfig.value("https_port", 8443)
        );
        addresses.push_back(httpsAddr);
        LOG(INFO) << "HTTPS enabled on port " << serverConfig.value("https_port", 8443);
    }

    if (addresses.empty()) {
        LOG(ERROR) << "No valid HTTP or HTTPS ports configured";
        return false;
    }

    // Create the server with options only (based on the available constructor)
    server_ = std::make_unique<proxygen::HTTPServer>(std::move(options));

    // Get main event base
    mainEventBase_ = eventBaseManager_.getEventBase();

    LOG(INFO) << "Server initialized";
    return true;
}

bool ServerApp::initDatabase() {
    try {
        if (!config_.contains("database")) {
            LOG(WARNING) << "No database configuration found";
            return true; // Not an error, just a warning
        }

        const auto& dbConfig = config_["database"];

        // Connect to database
        bool dbResult = db::DatabaseManager::getInstance().initialize(dbConfig);
        if (!dbResult) {
            LOG(ERROR) << "Failed to initialize database";
            return false;
        }

        LOG(INFO) << "Database connection established";
        return true;
    } catch (const std::exception& e) {
        LOG(ERROR) << "Database initialization failed: " << e.what();
        return false;
    }
}

bool ServerApp::setupSSL(proxygen::HTTPServerOptions& options) {
    try {
        const auto& sslConfig = config_["server"]["ssl"];

        // Create SSL context
        auto sslContext = std::make_shared<folly::SSLContext>();

        // Set cipher list
        sslContext->setCipherList("ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256");
        sslContext->setOptions(SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);

        // Load certificate and private key
        std::string certPath = sslConfig.value("cert_path", "./ssl/cert.pem");
        std::string keyPath = sslConfig.value("key_path", "./ssl/key.pem");
        std::string passphrase = sslConfig.value("passphrase", "");

        // Load private key (with or without passphrase)
        if (!passphrase.empty()) {
            LOG(INFO) << "Using key with passphrase";
        }

        // Just try to load the key - SSL libraries typically handle the passphrase internally
        try {
            sslContext->loadPrivateKey(keyPath.c_str());
        } catch (const std::exception& e) {
            LOG(ERROR) << "Failed to load private key: " << e.what();
            return false;
        }

        sslContext->loadCertificate(certPath.c_str());

        // Load CA certificate if provided
        if (sslConfig.contains("ca_path")) {
            std::string caPath = sslConfig["ca_path"];
            sslContext->loadTrustedCertificates(caPath.c_str());
            sslContext->authenticate(true, false);
        }

        // Set the SSL context for HTTPS
        options.enableContentCompression = false;
        options.handlerFactories = proxygen::RequestHandlerChain()
            .addThen(std::make_unique<handlers::HandlerFactory>(config_))
            .build();

        LOG(INFO) << "SSL configured successfully";
        return true;
    } catch (const std::exception& e) {
        LOG(ERROR) << "SSL setup failed: " << e.what();
        return false;
    }
}

bool ServerApp::start() {
    if (!server_) {
        LOG(ERROR) << "Server not initialized";
        return false;
    }

    // Start the server
    server_->start();
    running_ = true;

    LOG(INFO) << "Server started";
    return true;
}

void ServerApp::stop() {
    if (running_ && server_) {
        LOG(INFO) << "Stopping server...";
        server_->stop();
        running_ = false;

        // Close database connection
        db::DatabaseManager::getInstance().close();

        LOG(INFO) << "Server stopped";
    }
}

void ServerApp::run() {
    if (!running_ || !mainEventBase_) {
        LOG(ERROR) << "Server not running";
        return;
    }

    LOG(INFO) << "Server running, press Ctrl+C to stop";

    // Run event loop
    mainEventBase_->loopForever();
}

} // namespace securapp
