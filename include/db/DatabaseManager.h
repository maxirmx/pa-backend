#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <postgresql/libpq-fe.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace securapp {
namespace db {

class DatabaseManager {
public:
    // Singleton instance
    static DatabaseManager& getInstance();

    // Initialize connection from config
    bool initialize(const json& dbConfig);

    // Close connection
    void close();

    // Check if connection is active
    bool isConnected() const;

    // Execute a query that doesn't return any results
    bool execute(const std::string& query);

    // Execute a query with parameters that doesn't return results
    bool executeParams(const std::string& query, const std::vector<std::string>& params);

    // Execute a query and return results as JSON
    json executeQuery(const std::string& query);

    // Execute a parameterized query and return results as JSON
    json executeQueryParams(const std::string& query, const std::vector<std::string>& params);

    // Begin transaction
    bool beginTransaction();

    // Commit transaction
    bool commitTransaction();

    // Rollback transaction
    bool rollbackTransaction();

private:
    // Private constructor for singleton
    DatabaseManager();

    // Prevent copying
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // Database connection
    PGconn* conn_;

    // Connection parameters
    std::string host_;
    std::string port_;
    std::string user_;
    std::string password_;
    std::string dbname_;
    std::string sslMode_;

    // Helper to convert PGresult to JSON
    json resultToJson(PGresult* result);
};

} // namespace db
} // namespace securapp
