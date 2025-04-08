#pragma once

#include "handlers/BaseHandler.h"

namespace securapp {
namespace handlers {

class HealthCheckHandler : public BaseHandler {
public:
    HealthCheckHandler() = default;
    ~HealthCheckHandler() override = default;

protected:
    void handleRequest() override;
};

} // namespace handlers
} // namespace securapp
