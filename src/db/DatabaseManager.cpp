#include "db/DatabaseManager.h"
#include <glog/logging.h>

namespace securapp {
namespace db {

DatabaseManager::DatabaseManager() : conn_(nullptr) {}

DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::initialize(const json& dbConfig) {
    try {
        // Extract config values
        host_ = dbConfig.value("host", "localhost");
        port_ = dbConfig.value("port", "5432");
        user_ = dbConfig.value("user", "postgres");
        password_ = dbConfig.value("password", "");
        dbname_ = dbConfig.value("dbname", "postgres");
        sslMode_ = dbConfig.value("ssl_mode", "prefer");

        // Construct connection string
        std::string connStr =
            "host=" + host_ + " " +
            "port=" + port_ + " " +
            "user=" + user_ + " " +
            "password=" + password_ + " " +
            "dbname=" + dbname_ + " " +
            "sslmode=" + sslMode_;

        // Connect to the database
        conn_ = PQconnectdb(connStr.c_str());

        // Check connection status
        if (PQstatus(conn_) != CONNECTION_OK) {
            LOG(ERROR) << "Connection to database failed: " << PQerrorMessage(conn_);
            close();
            return false;
        }

        LOG(INFO) << "Successfully connected to PostgreSQL database " << dbname_;
        return true;
    }
    catch (const std::exception& e) {
        LOG(ERROR) << "Database initialization error: " << e.what();
        return false;
    }
}

void DatabaseManager::close() {
    if (conn_) {
        PQfinish(conn_);
        conn_ = nullptr;
        LOG(INFO) << "Database connection closed";
    }
}

bool DatabaseManager::isConnected() const {
    return conn_ && (PQstatus(conn_) == CONNECTION_OK);
}

bool DatabaseManager::execute(const std::string& query) {
    if (!isConnected()) {
        LOG(ERROR) << "Cannot execute query: no connection";
        return false;
    }

    PGresult* result = PQexec(conn_, query.c_str());
    ExecStatusType status = PQresultStatus(result);

    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
        LOG(ERROR) << "Query execution failed: " << PQerrorMessage(conn_);
        PQclear(result);
        return false;
    }

    PQclear(result);
    return true;
}

bool DatabaseManager::executeParams(const std::string& query, const std::vector<std::string>& params) {
    if (!isConnected()) {
        LOG(ERROR) << "Cannot execute parameterized query: no connection";
        return false;
    }

    // Convert string parameters to char* array
    std::vector<const char*> paramValues;
    for (const auto& param : params) {
        paramValues.push_back(param.c_str());
    }

    PGresult* result = PQexecParams(
        conn_,
        query.c_str(),
        static_cast<int>(params.size()),
        nullptr,  // param types
        paramValues.data(),
        nullptr,  // param lengths
        nullptr,  // param formats
        0  // result format (0 = text)
    );

    ExecStatusType status = PQresultStatus(result);

    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
        LOG(ERROR) << "Parameterized query execution failed: " << PQerrorMessage(conn_);
        PQclear(result);
        return false;
    }

    PQclear(result);
    return true;
}

json DatabaseManager::executeQuery(const std::string& query) {
    if (!isConnected()) {
        LOG(ERROR) << "Cannot execute query: no connection";
        return json::array();
    }

    PGresult* result = PQexec(conn_, query.c_str());
    ExecStatusType status = PQresultStatus(result);

    if (status != PGRES_TUPLES_OK) {
        LOG(ERROR) << "Query execution failed: " << PQerrorMessage(conn_);
        PQclear(result);
        return json::array();
    }

    json resultJson = resultToJson(result);
    PQclear(result);
    return resultJson;
}

json DatabaseManager::executeQueryParams(const std::string& query, const std::vector<std::string>& params) {
    if (!isConnected()) {
        LOG(ERROR) << "Cannot execute parameterized query: no connection";
        return json::array();
    }

    // Convert string parameters to char* array
    std::vector<const char*> paramValues;
    for (const auto& param : params) {
        paramValues.push_back(param.c_str());
    }

    PGresult* result = PQexecParams(
        conn_,
        query.c_str(),
        static_cast<int>(params.size()),
        nullptr,  // param types
        paramValues.data(),
        nullptr,  // param lengths
        nullptr,  // param formats
        0  // result format (0 = text)
    );

    ExecStatusType status = PQresultStatus(result);

    if (status != PGRES_TUPLES_OK) {
        LOG(ERROR) << "Parameterized query execution failed: " << PQerrorMessage(conn_);
        PQclear(result);
        return json::array();
    }

    json resultJson = resultToJson(result);
    PQclear(result);
    return resultJson;
}

bool DatabaseManager::beginTransaction() {
    return execute("BEGIN TRANSACTION");
}

bool DatabaseManager::commitTransaction() {
    return execute("COMMIT");
}

bool DatabaseManager::rollbackTransaction() {
    return execute("ROLLBACK");
}

json DatabaseManager::resultToJson(PGresult* result) {
    json jsonArray = json::array();

    // Get number of rows and columns
    int rows = PQntuples(result);
    int cols = PQnfields(result);

    // Get column names
    std::vector<std::string> columnNames;
    for (int col = 0; col < cols; col++) {
        columnNames.push_back(PQfname(result, col));
    }

    // Iterate through rows
    for (int row = 0; row < rows; row++) {
        json jsonRow = json::object();

        // Iterate through columns
        for (int col = 0; col < cols; col++) {
            // Check if the value is NULL
            if (PQgetisnull(result, row, col)) {
                jsonRow[columnNames[col]] = nullptr;
            } else {
                // Get the value as a string
                const char* value = PQgetvalue(result, row, col);
                jsonRow[columnNames[col]] = value;
            }
        }

        jsonArray.push_back(jsonRow);
    }

    return jsonArray;
}

} // namespace db
} // namespace securapp
