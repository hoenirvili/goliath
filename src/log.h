#pragma once

#include <string>
#include <ostream>

namespace Log
{
	enum class level : std::uint8_t { 
		error,		// error for logging error messages
		warning,	// warning for logging warning messages
		info		// info for logging information messages
	};
	
	void init(std::ostream* os) noexcept;
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