#pragma once

#include "handlers/BaseHandler.h"

namespace securapp {
namespace handlers {

class NotFoundHandler : public BaseHandler {
public:
    NotFoundHandler() = default;
    ~NotFoundHandler() override = default;

protected:
    void handleRequest() override;
};

} // namespace handlers
} // namespace securapp
