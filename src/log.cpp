#include "log.h"
#include <string>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <cstdarg>

using namespace std;

namespace Log
{

	/**
	* m_prefix list of level and their string representation
	*/
	static unordered_map<level, string> prefix =
	{
		{ level::error, "ERROR" },
		{ level::warning, "WARNING" },
		{ level::info, "INFO" },
	};

	static unique_ptr<ostream> out;

	void init(ostream* os) noexcept
	{
		out.reset(os);
	}

	void write(
		level l,
		const char *file,
		const int line,
		const char *function,
		const char *format,
		...
	)
	{
		if (!out)
			return;

		auto _prefix = "|" + prefix[l] + "|";
		file = strrchr(file, '\\') ? strrchr(file, '\\') + 1 : file;
		va_list list;
		va_start(list, format);
		auto len = vsnprintf(NULL, 0, format, list) + 1;
		auto message = make_unique<char[]>(len);
		vsnprintf(message.get(), len, format, list);
		va_end(list);
		(*out)
			<< file
			<< ":"
			<< line
			<< ":"
			<< function
			<< " "
			<< _prefix
			<< " "
			<< message.get()
			<< std::endl;
	}

};