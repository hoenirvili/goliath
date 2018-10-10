#pragma once

#include "cfgtrace/error/error.h"
#include <ostream>

namespace logger
{
enum class level : uint8_t { error, warning, info };

PRIVATE_API void init(std::ostream *w) noexcept;

PRIVATE_API void write(level l, const char *file, const int line, const char *function, const char *format, ...);

PRIVATE_API void clean() noexcept;

}; // namespace logger

#define logger_info(format, ...) \
    logger::write(logger::level::info, __FILE__, __LINE__, __FUNCTION__, format, __VA_ARGS__)

#define logger_error(format, ...) \
    logger::write(logger::level::error, __FILE__, __LINE__, __FUNCTION__, format, __VA_ARGS__)

#define logger_warning(format, ...) \
    logger::write(logger::level::warning, __FILE__, __LINE__, __FUNCTION__, format, __VA_ARGS__)