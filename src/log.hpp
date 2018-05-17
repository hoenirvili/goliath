#pragma once

#include <string>

namespace Log {

enum class level : std::uint8_t { error, warning, info };

/**
 * init initialises the internal writer
 */
void init(std::ostream* os) noexcept;

/**
* write will format and write the message to the internal writer
* if the internal writer is has not been initliased this will throw
*/
void write(level l, const char *file, const int line, 
			const char *function, const char *format, ...);

};

#define log_info(format, ...) \
	Log::write(Log::level::info, __FILE__, __LINE__, __FUNCTION__, format, __VA_ARGS__)

#define log_error(format, ...) \
	Log::write(Log::level::error, __FILE__, __LINE__, __FUNCTION__, format, __VA_ARGS__)

#define log_warning(format, ...) \
	Log::write(Log::level::warning, __FILE__, __LINE__, __FUNCTION__, format, __VA_ARGS__)

#define log_init(writer) \
	Log::init(writer)