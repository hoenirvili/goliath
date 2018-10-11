#pragma once

#include <string>

namespace format
{
template <typename... Args>
std::string string(const char *format, Args... args)
{
    std::size_t size = std::snprintf(nullptr, 0, format, args...) + 1;
    auto buf = std::make_unique<char[]>(size);
    std::snprintf(buf.get(), size, format, args...);
    return std::string(buf.get());
}

}; // namespace format
