#include "todo_controller.hpp"

TodoController::TodoController(std::shared_ptr<TodoService> service)
    : service_(std::move(service)) {
    init_routes();
}

void TodoController::init_routes() {
    // GET / (when mounted at /api/todos, resolves to GET /api/todos)
    router_.get("/", [this](const twf::Request&) {
        auto items = service_->get_all();
        return twf::Response::json(items);
    });

    // POST / (when mounted at /api/todos, resolves to POST /api/todos)
    router_.post("/", [this](const twf::Request& req) {
        auto dto = req.json<twf::CreateTodoDto>();
        if (!dto) {
            return twf::Response::text("Invalid JSON payload: " + dto.error().message, 
                                       twf::StatusCode::BadRequest);
        }

        auto item = service_->create(dto->title);
        return twf::Response::json(item, twf::StatusCode::Created);
    });
}
