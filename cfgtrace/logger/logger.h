#pragma once

#include "cfgtrace/error/error.h"

#include <ostream>

namespace logger
{
enum class level : uint8_t { error, warning, info };

bool is_writer_set() noexcept;

void set_writer(std::ostream *w);

void write(level l, const char *file, const int line, const char *function, const char *format, ...) noexcept;

void unset_writer() noexcept;

}; // namespace logger

#define logger_info(format, ...) \
    logger::write(logger::level::info, __FILE__, __LINE__, __FUNCTION__, format, __VA_ARGS__)

#define logger_error(format, ...) \
    logger::write(logger::level::error, __FILE__, __LINE__, __FUNCTION__, format, __VA_ARGS__)

#define logger_warning(format, ...) \
    logger::write(logger::level::warning, __FILE__, __LINE__, __FUNCTION__, format, __VA_ARGS__)