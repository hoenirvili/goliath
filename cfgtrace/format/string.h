#pragma once

#include <string_view>

namespace format
{
template <typename... Args>
std::string string(std::string_view format, Args... args)
{
    std::size_t size = std::snprintf(nullptr, 0, format.data(), args...) + 1;
    auto buf = std::make_unique<char[]>(size);
    std::snprintf(buf.get(), size, format.data(), args...);
    return std::string(buf.get(), buf.get() + size - 1);
}

}; // namespace format
