#pragma once

#include "twf/core/types.hpp"
#include "twf/server/request.hpp"
#include "twf/server/response.hpp"
#include "twf/server/router.hpp"
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace twf {

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

    Server& patch(std::string_view path, Handler handler) {
        return route(Method::PATCH, path, std::move(handler));
    }

    Server& options(std::string_view path, Handler handler) {
        return route(Method::OPTIONS, path, std::move(handler));
    }

    /// Mount a Router with a path prefix (e.g. app.use("/api/todos", todo_router))
    Server& use(std::string_view prefix, const Router& router);

    /// Mount a Router at root path
    Server& use(const Router& router);

    /// Serve static directory mounted at URL prefix (e.g. app.serve_static("/", "./public"))
    Server& serve_static(std::string_view mount_prefix, std::string_view directory_path);

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
