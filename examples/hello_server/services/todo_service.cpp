#include "todo_service.hpp"
#include <algorithm>

TodoService::TodoService() {
    todos_ = {
        {1, "Learn Modern C++23 Features", true},
        {2, "Build twf Full-Stack Architecture", true},
        {3, "Automatic JSON Reflection with Glaze", true},
        {4, "Compile C++ to WebAssembly (Frontend)", false}
    };
    next_id_ = 5;
}

std::vector<twf::TodoItem> TodoService::get_all() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return todos_;
}

std::optional<twf::TodoItem> TodoService::get_by_id(int id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::find_if(todos_.begin(), todos_.end(), [id](const twf::TodoItem& item) {
        return item.id == id;
    });
    if (it != todos_.end()) {
        return *it;
    }
    return std::nullopt;
}

twf::TodoItem TodoService::create(std::string_view title) {
    std::lock_guard<std::mutex> lock(mutex_);
    twf::TodoItem item{next_id_++, std::string(title), false};
    todos_.push_back(item);
    return item;
}

bool TodoService::toggle_complete(int id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::find_if(todos_.begin(), todos_.end(), [id](twf::TodoItem& item) {
        return item.id == id;
    });
    if (it != todos_.end()) {
        it->completed = !it->completed;
        return true;
    }
    return false;
}

bool TodoService::remove(int id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::remove_if(todos_.begin(), todos_.end(), [id](const twf::TodoItem& item) {
        return item.id == id;
    });
    if (it != todos_.end()) {
        todos_.erase(it, todos_.end());
        return true;
    }
    return false;
}
