#pragma once

#include "handlers/BaseHandler.h"

namespace securapp {
namespace handlers {

class ApiHandler : public BaseHandler {
public:
    explicit ApiHandler(const json& config);
    ~ApiHandler() override = default;

protected:
    void handleRequest() override;

private:
    // Handles different API endpoints
    void handleUsersEndpoint();
    void handleAuthEndpoint();

    // Configuration
    json config_;
};

} // namespace handlers
} // namespace securapp
