#include "status_controller.hpp"

StatusController::StatusController() {
    init_routes();
}

void StatusController::init_routes() {
    // GET / (when mounted at /api/status, resolves to GET /api/status)
    router_.get("/", [](const twf::Request&) {
        StatusInfo info;
        return twf::Response::json(info);
    });
}
