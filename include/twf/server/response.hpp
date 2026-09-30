#pragma once

#include "twf/core/types.hpp"
#include <format>
#include <string>
#include <string_view>
#include <vector>

namespace twf {

/// Represents an outgoing HTTP Response
class Response {
public:
    Response() = default;
    explicit Response(StatusCode status_code) : status_(status_code) {}

    /// Set status code
    Response& status(StatusCode code) noexcept {
        status_ = code;
        return *this;
    }

    Response& status(int code) noexcept {
        status_ = static_cast<StatusCode>(code);
        return *this;
    }

    /// Add or update header
    Response& header(std::string key, std::string value) {
        headers_.emplace_back(std::move(key), std::move(value));
        return *this;
    }

    /// Set body content
    Response& body(std::string body_text) {
        body_ = std::move(body_text);
        return *this;
    }

    /// Factory for plain text response
    static Response text(std::string_view text, StatusCode code = StatusCode::OK) {
        Response res(code);
        res.header("Content-Type", "text/plain; charset=utf-8");
        res.body(std::string(text));
        return res;
    }

    /// Factory for HTML response
    static Response html(std::string_view html, StatusCode code = StatusCode::OK) {
        Response res(code);
        res.header("Content-Type", "text/html; charset=utf-8");
        res.body(std::string(html));
        return res;
    }

    /// Factory for JSON response
    static Response json(std::string_view json_str, StatusCode code = StatusCode::OK) {
        Response res(code);
        res.header("Content-Type", "application/json; charset=utf-8");
        res.body(std::string(json_str));
        return res;
    }

    /// Accessors
    StatusCode status_code() const noexcept { return status_; }
    const std::string& get_body() const noexcept { return body_; }
    const std::vector<std::pair<std::string, std::string>>& headers() const noexcept { return headers_; }

    /// Serialize into valid HTTP/1.1 wire protocol string
    std::string serialize() const {
        std::string out;
        out.reserve(128 + body_.size());

        // HTTP/1.1 Status Line
        out += std::format("HTTP/1.1 {} {}\r\n", 
            static_cast<uint16_t>(status_), 
            status_code_to_reason(status_));

        // Content-Length
        out += std::format("Content-Length: {}\r\n", body_.size());

        // Custom Headers
        bool has_connection = false;
        for (const auto& [k, v] : headers_) {
            out += std::format("{}: {}\r\n", k, v);
            if (k == "Connection" || k == "connection") {
                has_connection = true;
            }
        }

        if (!has_connection) {
            out += "Connection: keep-alive\r\n";
        }

        // Server signature
        out += "Server: twf/0.1.0 (C++23)\r\n";

        // End of headers
        out += "\r\n";

        // Body
        out += body_;
        return out;
    }

private:
    StatusCode status_{StatusCode::OK};
    std::vector<std::pair<std::string, std::string>> headers_;
    std::string body_;
};

} // namespace twf
