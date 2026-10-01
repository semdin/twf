#include "twf/server/router.hpp"

namespace twf {

std::string join_paths(std::string_view prefix, std::string_view path) {
    if (prefix.empty()) {
        if (path.empty()) return "/";
        if (path.front() != '/') return "/" + std::string(path);
        return std::string(path);
    }

    std::string p(prefix);
    if (p.front() != '/') p.insert(p.begin(), '/');
    while (p.size() > 1 && p.back() == '/') p.pop_back();

    if (path.empty()) {
        return p;
    }

    std::string s(path);
    if (s.front() == '/') {
        if (p == "/") return s;
        return p + s;
    } else {
        if (p == "/") return "/" + s;
        return p + "/" + s;
    }
}

Router& Router::route(Method method, std::string_view path, Handler handler) {
    std::string normalized_path = path.empty() ? "/" : std::string(path);
    if (normalized_path.front() != '/') {
        normalized_path.insert(normalized_path.begin(), '/');
    }
    while (normalized_path.size() > 1 && normalized_path.back() == '/') {
        normalized_path.pop_back();
    }
    routes_.push_back(RouteEntry{method, std::move(normalized_path), std::move(handler)});
    return *this;
}

Router& Router::use(std::string_view prefix, const Router& sub_router) {
    for (const auto& entry : sub_router.routes()) {
        std::string joined = join_paths(prefix, entry.path);
        while (joined.size() > 1 && joined.back() == '/') {
            joined.pop_back();
        }
        routes_.push_back(RouteEntry{entry.method, std::move(joined), entry.handler});
    }
    return *this;
}

Router& Router::use(const Router& sub_router) {
    return use("", sub_router);
}

} // namespace twf
