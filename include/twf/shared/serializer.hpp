#pragma once

#include "twf/core/types.hpp"
#include <glaze/glaze.hpp>
#include <string>
#include <string_view>

namespace twf {

/// Serialize any C++ struct/container to JSON string using compile-time reflection
template <typename T>
Result<std::string> to_json(const T& value) {
    auto res = glz::write_json(value);
    if (!res) {
        return std::unexpected(Error{400, "JSON serialization error: " + glz::format_error(res.error(), "")});
    }
    return *res;
}

/// Deserialize JSON string into a strongly-typed C++ struct/container
template <typename T>
Result<T> from_json(std::string_view json_str) {
    auto res = glz::read_json<T>(json_str);
    if (!res) {
        return std::unexpected(Error{400, "JSON deserialization error: " + glz::format_error(res.error(), json_str)});
    }
    return *res;
}

} // namespace twf
