#include "cfgtrace/logger/logger.h"
#include <cstdarg>
#include <map>
#include <ostream>
#include <string>

namespace logger
{
/**
 *
 */
static std::ostream *out;

void init(std::ostream *os) noexcept
{
    if (!out)
        out = os;
    else
        delete os;
}

void write(level l, const char *file, const int line, const char *function, const char *format, ...)
{
    if (!out)
        return;

    const std::map<level, std::string> table = {
      {level::error, "ERROR"}, {level::warning, "WARNING"}, {level::info, "INFO"}};
    auto _prefix = "|" + table.at(l) + "|";
    file = strrchr(file, '\\') ? strrchr(file, '\\') + 1 : file;
    va_list list;
    va_start(list, format);
    auto len = vsnprintf(nullptr, 0, format, list) + 1;
    auto message = std::make_unique<char[]>(len);
    vsnprintf(message.get(), len, format, list);
    va_end(list);
    (*out) << file << ":" << line << ":" << function << " " << _prefix << " " << message.get() << std::endl;
}

void clean() noexcept
{
    delete out;
}

}; // namespace logger