#include "twf/server/server.hpp"
#include "controllers/todo_controller.hpp"
#include "controllers/status_controller.hpp"
#include "services/todo_service.hpp"

#include <iostream>
#include <format>
#include <memory>

int main() {
    twf::Server app;

    // 1. Dependency Injection: Initialize Services
    auto todo_service = std::make_shared<TodoService>();

    // 2. Initialize Controllers with injected dependencies
    TodoController todo_controller(todo_service);
    StatusController status_controller;

    // 3. Mount modular sub-routers
    app.use("/api/todos", todo_controller.router());
    app.use("/api/status", status_controller.router());

    // 4. Mount Static Assets
    // - Root serves index.html & static files from public/
    // - /wasm serves compiled C++ WebAssembly module (wasm_client.js & wasm_client.wasm)
    app.serve_static("/", "examples/hello_server/public");
    app.serve_static("/wasm", "examples/wasm_client");

    std::cout << "[twf] Starting modular C++23 server at http://127.0.0.1:8080 ...\n";
    auto result = app.listen("127.0.0.1", 8080);
    if (!result) {
        std::cerr << std::format("[twf Error] {}\n", result.error().message);
        return 1;
    }

    return 0;
}
