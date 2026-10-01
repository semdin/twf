#pragma once

#include "twf/server/router.hpp"
#include "services/todo_service.hpp"
#include <memory>

class TodoController {
public:
    explicit TodoController(std::shared_ptr<TodoService> service);

    /// Access the controller's dedicated sub-router
    const twf::Router& router() const noexcept { return router_; }

private:
    void init_routes();

    std::shared_ptr<TodoService> service_;
    twf::Router router_;
};
