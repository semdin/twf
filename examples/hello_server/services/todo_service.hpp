#pragma once

#include "twf/shared/models.hpp"
#include <mutex>
#include <vector>
#include <optional>
#include <string_view>

class TodoService {
public:
    TodoService();

    /// Retrieve all todo items (thread-safe)
    std::vector<twf::TodoItem> get_all() const;

    /// Retrieve a single todo item by ID
    std::optional<twf::TodoItem> get_by_id(int id) const;

    /// Create and persist a new todo item
    twf::TodoItem create(std::string_view title);

    /// Toggle completed state of a todo item
    bool toggle_complete(int id);

    /// Remove a todo item by ID
    bool remove(int id);

private:
    mutable std::mutex mutex_;
    std::vector<twf::TodoItem> todos_;
    int next_id_{1};
};
