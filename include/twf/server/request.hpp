#pragma once

#include "twf/core/types.hpp"
#include <algorithm>
#include <cctype>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace twf {

/// Case-insensitive comparator for HTTP header names
struct HeaderCaseInsensitiveHash {
    size_t operator()(std::string_view str) const noexcept {
        size_t h = 0;
        for (char c : str) {
            h = h * 31 + static_cast<unsigned char>(std::tolower(c));
        }
        return h;
    }
};

struct HeaderCaseInsensitiveEqual {
    bool operator()(std::string_view a, std::string_view b) const noexcept {
        if (a.size() != b.size()) return false;
        return std::equal(a.begin(), a.end(), b.begin(), [](char c1, char c2) {
            return std::tolower(static_cast<unsigned char>(c1)) ==
                   std::tolower(static_cast<unsigned char>(c2));
        });
    }
};

/// Represents an incoming HTTP Request
class Request {
public:
    using HeaderMap = std::unordered_map<
        std::string, 
        std::string, 
        HeaderCaseInsensitiveHash, 
        HeaderCaseInsensitiveEqual
    >;

    Method method{Method::UNKNOWN};
    std::string path;
    std::string raw_query;
    std::string version{"HTTP/1.1"};
    HeaderMap headers;
    std::string body;

    /// Case-insensitive header lookup
    std::optional<std::string_view> get_header(std::string_view key) const noexcept {
        auto it = headers.find(std::string(key));
        if (it != headers.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    /// Extract query parameter by key, e.g. /search?q=cpp -> get_query("q") == "cpp"
    std::optional<std::string> get_query(std::string_view key) const {
        if (raw_query.empty()) return std::nullopt;

        size_t start = 0;
        while (start < raw_query.size()) {
            size_t end = raw_query.find('&', start);
            if (end == std::string::npos) end = raw_query.size();

            std::string_view param(&raw_query[start], end - start);
            size_t eq_pos = param.find('=');
            if (eq_pos != std::string_view::npos) {
                std::string_view k = param.substr(0, eq_pos);
                if (k == key) {
                    return std::string(param.substr(eq_pos + 1));
                }
            } else if (param == key) {
                return std::string("");
            }

            start = end + 1;
        }
        return std::nullopt;
    }
};

} // namespace twf
