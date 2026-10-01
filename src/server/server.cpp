#include "twf/server/server.hpp"
#include "twf/core/buffer.hpp"

#include <iostream>
#include <format>
#include <thread>
#include <mutex>
#include <vector>
#include <sstream>
#include <charconv>
#include <filesystem>
#include <cctype>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    using socket_t = SOCKET;
    using socklen_t = int;
    constexpr socket_t INVALID_SOCK = INVALID_SOCKET;
    constexpr int SOCK_ERR = SOCKET_ERROR;
    #define CLOSE_SOCK(s) closesocket(s)
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    using socket_t = int;
    constexpr socket_t INVALID_SOCK = -1;
    constexpr int SOCK_ERR = -1;
    #define CLOSE_SOCK(s) close(s)
#endif

namespace twf {

namespace {

// Helper to trim leading/trailing whitespace
std::string_view trim(std::string_view str) {
    while (!str.empty() && (str.front() == ' ' || str.front() == '\t' || str.front() == '\r')) {
        str.remove_prefix(1);
    }
    while (!str.empty() && (str.back() == ' ' || str.back() == '\t' || str.back() == '\r')) {
        str.remove_suffix(1);
    }
    return str;
}

// Parse raw HTTP request buffer into twf::Request object
std::optional<Request> parse_http_request(const Buffer& buf) {
    std::string_view raw = buf.string_view();
    size_t header_end = raw.find("\r\n\r\n");
    if (header_end == std::string_view::npos) {
        header_end = raw.find("\n\n");
        if (header_end == std::string_view::npos) return std::nullopt;
    }

    Request req;
    std::string_view header_section = raw.substr(0, header_end);
    
    // Split into lines
    size_t line_start = 0;
    bool is_first_line = true;

    while (line_start < header_section.size()) {
        size_t line_end = header_section.find('\n', line_start);
        if (line_end == std::string_view::npos) line_end = header_section.size();

        std::string_view line = trim(header_section.substr(line_start, line_end - line_start));
        line_start = line_end + 1;

        if (line.empty()) continue;

        if (is_first_line) {
            is_first_line = false;
            // e.g. "GET /hello?name=world HTTP/1.1"
            size_t m_end = line.find(' ');
            if (m_end == std::string_view::npos) return std::nullopt;

            req.method = string_to_method(line.substr(0, m_end));

            size_t uri_end = line.find(' ', m_end + 1);
            if (uri_end == std::string_view::npos) {
                uri_end = line.size();
            } else {
                req.version = std::string(trim(line.substr(uri_end + 1)));
            }

            std::string_view full_uri = line.substr(m_end + 1, uri_end - (m_end + 1));
            size_t q_mark = full_uri.find('?');
            if (q_mark != std::string_view::npos) {
                req.path = std::string(full_uri.substr(0, q_mark));
                req.raw_query = std::string(full_uri.substr(q_mark + 1));
            } else {
                req.path = std::string(full_uri);
            }
        } else {
            // Header line: "Key: Value"
            size_t colon = line.find(':');
            if (colon != std::string_view::npos) {
                std::string key(trim(line.substr(0, colon)));
                std::string value(trim(line.substr(colon + 1)));
                req.headers[std::move(key)] = std::move(value);
            }
        }
    }

    // Body handling
    size_t body_start = (raw.find("\r\n\r\n") != std::string_view::npos) ? header_end + 4 : header_end + 2;
    if (body_start < raw.size()) {
        req.body = std::string(raw.substr(body_start));
    }

    return req;
}

std::string_view get_mime_type(const std::filesystem::path& file_path) {
    auto ext = file_path.extension().string();
    for (char& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    if (ext == ".html" || ext == ".htm") return "text/html; charset=utf-8";
    if (ext == ".css") return "text/css; charset=utf-8";
    if (ext == ".js" || ext == ".mjs") return "application/javascript; charset=utf-8";
    if (ext == ".json") return "application/json; charset=utf-8";
    if (ext == ".wasm") return "application/wasm";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif") return "image/gif";
    if (ext == ".svg") return "image/svg+xml";
    if (ext == ".ico") return "image/x-icon";
    if (ext == ".txt") return "text/plain; charset=utf-8";
    if (ext == ".xml") return "application/xml; charset=utf-8";
    if (ext == ".pdf") return "application/pdf";
    if (ext == ".mp4") return "video/mp4";
    if (ext == ".webm") return "video/webm";
    return "application/octet-stream";
}

} // anonymous namespace

struct Server::Impl {
    struct StaticMount {
        std::string mount_prefix;
        std::string directory_path;
    };

    socket_t listen_socket{INVALID_SOCK};
    std::atomic<bool> is_running{false};
    std::unordered_map<std::string, Handler> routes;
    std::vector<StaticMount> static_mounts;
    Handler not_found_handler{[](const Request& req) {
        return Response::html(
            std::format("<!DOCTYPE html><html><body style='font-family:system-ui;padding:2rem;background:#111;color:#eee'>"
                        "<h2>404 Not Found</h2><p>No route matches path: <code>{}</code></p></body></html>", req.path),
            StatusCode::NotFound
        );
    }};

    static std::string make_route_key(Method method, std::string_view path) {
        std::string p(path);
        if (p.empty() || p.front() != '/') p.insert(p.begin(), '/');
        while (p.size() > 1 && p.back() == '/') p.pop_back();
        return std::format("{}|{}", method_to_string(method), p);
    }

    std::optional<Response> try_serve_static(const Request& req) const {
        if (req.method != Method::GET && req.method != Method::HEAD) {
            return std::nullopt;
        }

        for (const auto& mount : static_mounts) {
            const std::string& prefix = mount.mount_prefix;
            std::string_view req_path = req.path;

            bool matches = false;
            std::string_view rel_subpath;

            if (prefix == "/") {
                matches = true;
                rel_subpath = req_path;
                if (!rel_subpath.empty() && rel_subpath.front() == '/') {
                    rel_subpath.remove_prefix(1);
                }
            } else if (req_path == prefix) {
                matches = true;
                rel_subpath = "";
            } else if (req_path.starts_with(prefix) && req_path.size() > prefix.size() && req_path[prefix.size()] == '/') {
                matches = true;
                rel_subpath = req_path.substr(prefix.size() + 1);
            }

            if (!matches) continue;

            std::error_code ec;
            std::filesystem::path base = std::filesystem::weakly_canonical(mount.directory_path, ec);
            if (ec || !std::filesystem::exists(base, ec)) continue;

            std::filesystem::path target = std::filesystem::weakly_canonical(base / rel_subpath, ec);
            if (ec) continue;

            // Directory traversal prevention
            auto [root_end, _] = std::mismatch(base.begin(), base.end(), target.begin());
            if (root_end != base.end()) {
                return Response::text("Forbidden: directory traversal detected", StatusCode::Forbidden);
            }

            if (std::filesystem::is_directory(target, ec)) {
                target /= "index.html";
            }

            if (std::filesystem::is_regular_file(target, ec)) {
                auto mime = get_mime_type(target);
                return Response::file(target.string(), mime);
            }
        }

        return std::nullopt;
    }
};

Server::Server() : pimpl_(std::make_unique<Impl>()) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
}

Server::~Server() {
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

Server::Server(Server&&) noexcept = default;
Server& Server::operator=(Server&&) noexcept = default;

bool Server::is_running() const noexcept {
    return pimpl_ ? pimpl_->is_running.load() : false;
}

Server& Server::route(Method method, std::string_view path, Handler handler) {
    pimpl_->routes[Impl::make_route_key(method, path)] = std::move(handler);
    return *this;
}

Server& Server::use(std::string_view prefix, const Router& router) {
    for (const auto& entry : router.routes()) {
        std::string joined = join_paths(prefix, entry.path);
        route(entry.method, joined, entry.handler);
    }
    return *this;
}

Server& Server::use(const Router& router) {
    return use("", router);
}

Server& Server::serve_static(std::string_view mount_prefix, std::string_view directory_path) {
    std::string prefix(mount_prefix);
    if (prefix.empty() || prefix.front() != '/') prefix.insert(prefix.begin(), '/');
    while (prefix.size() > 1 && prefix.back() == '/') prefix.pop_back();
    pimpl_->static_mounts.push_back(Impl::StaticMount{std::move(prefix), std::string(directory_path)});
    return *this;
}

Server& Server::not_found(Handler handler) {
    pimpl_->not_found_handler = std::move(handler);
    return *this;
}

void Server::stop() {
    if (pimpl_) {
        pimpl_->is_running.store(false);
        if (pimpl_->listen_socket != INVALID_SOCK) {
            CLOSE_SOCK(pimpl_->listen_socket);
            pimpl_->listen_socket = INVALID_SOCK;
        }
    }
}

Result<void> Server::listen(std::string_view host, int port) {
    pimpl_->listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (pimpl_->listen_socket == INVALID_SOCK) {
        return std::unexpected(Error::from_system(0, "Failed to create listening socket"));
    }

    // Enable SO_REUSEADDR
    int opt = 1;
#ifdef _WIN32
    setsockopt(pimpl_->listen_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
    setsockopt(pimpl_->listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(static_cast<uint16_t>(port));

    if (host == "0.0.0.0" || host.empty()) {
        server_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_pton(AF_INET, std::string(host).c_str(), &server_addr.sin_addr);
    }

    if (bind(pimpl_->listen_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) == SOCK_ERR) {
        stop();
        return std::unexpected(Error::from_system(0, std::format("Failed to bind socket on {}:{}", host, port)));
    }

    if (::listen(pimpl_->listen_socket, SOMAXCONN) == SOCK_ERR) {
        stop();
        return std::unexpected(Error::from_system(0, "Failed to listen on socket"));
    }

    pimpl_->is_running.store(true);
    std::cout << std::format("[twf::Server] Listening at http://{}:{}/\n", host.empty() ? "localhost" : host, port);

    // Accept loop
    while (pimpl_->is_running.load()) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        socket_t client_sock = accept(pimpl_->listen_socket, reinterpret_cast<sockaddr*>(&client_addr), &client_len);

        if (client_sock == INVALID_SOCK) {
            if (!pimpl_->is_running.load()) break;
            continue;
        }

        // Handle client connection concurrently in a separate thread
        std::thread([this, client_sock]() {
            Buffer recv_buf(4096);
            char temp[4096];

            int bytes_read = recv(client_sock, temp, sizeof(temp), 0);
            if (bytes_read > 0) {
                recv_buf.append(temp, bytes_read);

                auto opt_req = parse_http_request(recv_buf);
                if (opt_req) {
                    Request req = std::move(*opt_req);

                    // Read remaining body bytes if needed (reverse proxies like Caddy send body in chunks)
                    auto cl_header = req.get_header("content-length");
                    if (cl_header) {
                        size_t expected_len = 0;
                        try {
                            expected_len = std::stoull(std::string(*cl_header));
                        } catch (...) {}

                        while (req.body.size() < expected_len) {
                            int more = recv(client_sock, temp, sizeof(temp), 0);
                            if (more <= 0) break;
                            req.body.append(temp, more);
                        }
                    }

                    std::string key = Impl::make_route_key(req.method, req.path);

                    Response res;
                    auto it = pimpl_->routes.find(key);
                    if (it != pimpl_->routes.end()) {
                        res = it->second(req);
                    } else {
                        auto static_res = pimpl_->try_serve_static(req);
                        if (static_res) {
                            res = std::move(*static_res);
                        } else {
                            res = pimpl_->not_found_handler(req);
                        }
                    }

                    std::string wire_data = res.serialize();
                    send(client_sock, wire_data.data(), static_cast<int>(wire_data.size()), 0);
                }
            }

            CLOSE_SOCK(client_sock);
        }).detach();
    }

    return {};
}

} // namespace twf
