#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

namespace twf {

/// HTTP Methods supported by twf
enum class Method {
    UNKNOWN = 0,
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    OPTIONS,
    HEAD
};

/// Convert string representation to Method enum
constexpr Method string_to_method(std::string_view method_str) noexcept {
    if (method_str == "GET") return Method::GET;
    if (method_str == "POST") return Method::POST;
    if (method_str == "PUT") return Method::PUT;
    if (method_str == "DELETE") return Method::DELETE;
    if (method_str == "PATCH") return Method::PATCH;
    if (method_str == "OPTIONS") return Method::OPTIONS;
    if (method_str == "HEAD") return Method::HEAD;
    return Method::UNKNOWN;
}

/// Convert Method enum to string representation
constexpr std::string_view method_to_string(Method method) noexcept {
    switch (method) {
        case Method::GET: return "GET";
        case Method::POST: return "POST";
        case Method::PUT: return "PUT";
        case Method::DELETE: return "DELETE";
        case Method::PATCH: return "PATCH";
        case Method::OPTIONS: return "OPTIONS";
        case Method::HEAD: return "HEAD";
        default: return "UNKNOWN";
    }
}

/// HTTP Status Codes
enum class StatusCode : uint16_t {
    OK = 200,
    Created = 201,
    Accepted = 202,
    NoContent = 204,
    MovedPermanently = 301,
    Found = 302,
    NotModified = 304,
    BadRequest = 400,
    Unauthorized = 401,
    Forbidden = 403,
    NotFound = 404,
    MethodNotAllowed = 405,
    InternalServerError = 500,
    NotImplemented = 501,
    BadGateway = 502,
    ServiceUnavailable = 503
};

/// Reason phrases for HTTP status codes
constexpr std::string_view status_code_to_reason(StatusCode code) noexcept {
    switch (code) {
        case StatusCode::OK: return "OK";
        case StatusCode::Created: return "Created";
        case StatusCode::Accepted: return "Accepted";
        case StatusCode::NoContent: return "No Content";
        case StatusCode::MovedPermanently: return "Moved Permanently";
        case StatusCode::Found: return "Found";
        case StatusCode::NotModified: return "Not Modified";
        case StatusCode::BadRequest: return "Bad Request";
        case StatusCode::Unauthorized: return "Unauthorized";
        case StatusCode::Forbidden: return "Forbidden";
        case StatusCode::NotFound: return "Not Found";
        case StatusCode::MethodNotAllowed: return "Method Not Allowed";
        case StatusCode::InternalServerError: return "Internal Server Error";
        case StatusCode::NotImplemented: return "Not Implemented";
        case StatusCode::BadGateway: return "Bad Gateway";
        case StatusCode::ServiceUnavailable: return "Service Unavailable";
        default: return "Unknown";
    }
}

/// Framework Error representation using C++23 std::expected
struct Error {
    int code{0};
    std::string message;

    static Error from_system(int err, std::string_view context = "") {
        return Error{err, std::string(context) + " (System error code: " + std::to_string(err) + ")"};
    }
};

/// Modern C++23 Result type: Result<T, Error>
template <typename T>
using Result = std::expected<T, Error>;

} // namespace twf
