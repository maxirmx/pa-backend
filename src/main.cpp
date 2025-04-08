#include "ServerApp.h"
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <iostream>
#include <string>
#include <sys/stat.h>

// Define command line flags
DEFINE_string(config, "./config/server_config.json", "Path to server configuration file");
DEFINE_bool(verbose, false, "Enable verbose logging");
DEFINE_string(logdir, "./logs", "Directory for log files");

// Helper function to create directories
void createDirectories(const std::string& path) {
    // Split the path and create each directory level
    std::string current;

    for (char c : path) {
        if (c == '/') {
            if (!current.empty()) {
                // Create directory if it doesn't exist
                mkdir(current.c_str(), 0755);
            }
            current += c;
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        mkdir(current.c_str(), 0755);
    }
}

int main(int argc, char* argv[]) {
    // Initialize Google Flags
    gflags::SetUsageMessage("Secure Application Server using Proxygen and PostgreSQL");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    // Initialize Google Logging
    google::InitGoogleLogging(argv[0]);

    // Create log directory if it doesn't exist
    createDirectories(FLAGS_logdir);
    FLAGS_log_dir = FLAGS_logdir;

    // Set logging verbosity
    if (FLAGS_verbose) {
        FLAGS_minloglevel = 0;  // INFO and above
        FLAGS_v = 1;
    } else {
        FLAGS_minloglevel = 1;  // WARNING and above
        FLAGS_v = 0;
    }

    // Log to stderr as well
    FLAGS_alsologtostderr = true;

    LOG(INFO) << "Starting Secure Application Server";
    LOG(INFO) << "Using configuration file: " << FLAGS_config;

    try {
        // Create the server application
        securapp::ServerApp server;

        // Initialize the server
        LOG(INFO) << "Initializing server...";
        if (!server.initialize(FLAGS_config)) {
            LOG(ERROR) << "Failed to initialize server";
            return 1;
        }

        // Start the server
        LOG(INFO) << "Starting server...";
        if (!server.start()) {
            LOG(ERROR) << "Failed to start server";
            return 1;
        }

        // Run the server (blocks until server is stopped)
        server.run();

        LOG(INFO) << "Server shutdown complete";
        return 0;
    }
    catch (const std::exception& e) {
        LOG(ERROR) << "Server error: " << e.what();
        return 1;
    }
    catch (...) {
        LOG(ERROR) << "Unknown server error";
        return 1;
    }
}
