#pragma once

#include <string>
#include <vector>

namespace twf {

/// Shared Todo model used by both Backend and Frontend
struct TodoItem {
    int id{0};
    std::string title;
    bool completed{false};
};

/// Shared DTO for creating a new Todo
struct CreateTodoDto {
    std::string title;
};

/// Generic API response wrapper
template <typename T>
struct ApiResponse {
    bool success{true};
    std::string message;
    T data;
};

} // namespace twf
