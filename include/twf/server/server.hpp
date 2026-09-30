#pragma once

#include "twf/core/types.hpp"
#include "twf/server/request.hpp"
#include "twf/server/response.hpp"
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace twf {

using Handler = std::function<Response(const Request&)>;

/// High-performance HTTP server engine
class Server {
public:
    Server();
    ~Server();

    // Non-copyable, movable
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    Server(Server&&) noexcept;
    Server& operator=(Server&&) noexcept;

    /// Register a route for a specific method and path
    Server& route(Method method, std::string_view path, Handler handler);

    /// Convenience helpers
    Server& get(std::string_view path, Handler handler) {
        return route(Method::GET, path, std::move(handler));
    }

    Server& post(std::string_view path, Handler handler) {
        return route(Method::POST, path, std::move(handler));
    }

    Server& put(std::string_view path, Handler handler) {
        return route(Method::PUT, path, std::move(handler));
    }

    Server& del(std::string_view path, Handler handler) {
        return route(Method::DELETE, path, std::move(handler));
    }

    /// Set fallback 404 handler
    Server& not_found(Handler handler);

    /// Start listening on host and port (blocking call)
    Result<void> listen(std::string_view host, int port);

    /// Signal the server to stop
    void stop();

    /// Check if server is currently running
    bool is_running() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace twf
