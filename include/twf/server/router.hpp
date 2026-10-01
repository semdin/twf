#pragma once

#include "twf/core/types.hpp"
#include "twf/server/request.hpp"
#include "twf/server/response.hpp"
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace twf {

using Handler = std::function<Response(const Request&)>;

/// Represents a single registered route entry
struct RouteEntry {
    Method method;
    std::string path;
    Handler handler;
};

/// Joins a mount prefix and route path, handling slashes cleanly
std::string join_paths(std::string_view prefix, std::string_view path);

/// Modular Route Handler collection supporting sub-routing and path prefixes
class Router {
public:
    Router() = default;

    /// Register a route for a specific HTTP method
    Router& route(Method method, std::string_view path, Handler handler);

    /// HTTP method helpers
    Router& get(std::string_view path, Handler handler) {
        return route(Method::GET, path, std::move(handler));
    }

    Router& post(std::string_view path, Handler handler) {
        return route(Method::POST, path, std::move(handler));
    }

    Router& put(std::string_view path, Handler handler) {
        return route(Method::PUT, path, std::move(handler));
    }

    Router& del(std::string_view path, Handler handler) {
        return route(Method::DELETE, path, std::move(handler));
    }

    Router& patch(std::string_view path, Handler handler) {
        return route(Method::PATCH, path, std::move(handler));
    }

    Router& options(std::string_view path, Handler handler) {
        return route(Method::OPTIONS, path, std::move(handler));
    }

    /// Mount another router under a path prefix (e.g. router.use("/v1", v1_router))
    Router& use(std::string_view prefix, const Router& sub_router);

    /// Mount another router at current path level
    Router& use(const Router& sub_router);

    /// Access all compiled route entries
    const std::vector<RouteEntry>& routes() const noexcept { return routes_; }

private:
    std::vector<RouteEntry> routes_;
};

} // namespace twf
