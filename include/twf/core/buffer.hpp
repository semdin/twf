#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace twf {

/// Contiguous memory buffer optimized for network I/O
class Buffer {
public:
    Buffer() = default;
    explicit Buffer(size_t initial_capacity);

    /// Append raw bytes
    void append(const void* data, size_t len);

    /// Append string_view
    void append(std::string_view str);

    /// Clear contents without deallocating underlying memory
    void clear() noexcept;

    /// Reserve memory capacity
    void reserve(size_t capacity);

    /// Resize buffer
    void resize(size_t new_size);

    /// Direct pointer to data
    char* data() noexcept { return storage_.data(); }
    const char* data() const noexcept { return storage_.data(); }

    /// Current size in bytes
    size_t size() const noexcept { return storage_.size(); }

    /// Current allocated capacity
    size_t capacity() const noexcept { return storage_.capacity(); }

    /// Is empty?
    bool empty() const noexcept { return storage_.empty(); }

    /// View as string_view (zero allocation)
    std::string_view string_view() const noexcept {
        return std::string_view(storage_.data(), storage_.size());
    }

    /// Slice a sub-region as string_view (zero allocation)
    std::string_view slice(size_t offset, size_t length = std::string_view::npos) const noexcept {
        if (offset >= storage_.size()) return {};
        size_t available = storage_.size() - offset;
        size_t count = (length < available) ? length : available;
        return std::string_view(storage_.data() + offset, count);
    }

    /// View as byte span
    std::span<const uint8_t> as_bytes() const noexcept {
        return {reinterpret_cast<const uint8_t*>(storage_.data()), storage_.size()};
    }

private:
    std::vector<char> storage_;
};

} // namespace twf
