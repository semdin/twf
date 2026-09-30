#include "twf/core/buffer.hpp"
#include <cstring>

namespace twf {

Buffer::Buffer(size_t initial_capacity) {
    storage_.reserve(initial_capacity);
}

void Buffer::append(const void* data, size_t len) {
    if (!data || len == 0) return;
    const char* byte_ptr = static_cast<const char*>(data);
    storage_.insert(storage_.end(), byte_ptr, byte_ptr + len);
}

void Buffer::append(std::string_view str) {
    append(str.data(), str.size());
}

void Buffer::clear() noexcept {
    storage_.clear();
}

void Buffer::reserve(size_t capacity) {
    storage_.reserve(capacity);
}

void Buffer::resize(size_t new_size) {
    storage_.resize(new_size);
}

} // namespace twf
