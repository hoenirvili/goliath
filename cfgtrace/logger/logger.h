#pragma once

#include <ostream>
#include <string>

namespace logger
{
enum class level : uint8_t { error, warning, info };

void init(std::ostream *writer) noexcept;

void write(level l, const char *file, const int line, const char *function, const char *format, ...);
}; // namespace logger

#define logger_info(format, ...) \
    logger::write(logger::level::info, __FILE__, __LINE__, __FUNCTION__, format, __VA_ARGS__)

#define logger_error(format, ...) \
    logger::write(logger::level::error, __FILE__, __LINE__, __FUNCTION__, format, __VA_ARGS__)

#define logger_warning(format, ...) \
    logger::write(logger::level::warning, __FILE__, __LINE__, __FUNCTION__, format, __VA_ARGS__)

#define logger_init(writer) logger::init(writer)