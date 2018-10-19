#include "cfgtrace/logger/logger.h"
#include <cstdarg>
#include <map>
#include <ostream>
#include <stdexcept>
#include <string>

namespace logger
{
static std::ostream *w;

bool is_writer_set() noexcept
{
    return (w);
}

void set_writer(std::ostream *writer)
{
    w = writer;
}

void write(level l, const char *file, const int line, const char *function, const char *format, ...) noexcept
{
    if (!w)
        return;

    const std::map<level, std::string> table = {
      {level::error, "ERROR"}, {level::warning, "WARNING"}, {level::info, "INFO"}};
    auto _prefix = "|" + table.at(l) + "|";
    file = strrchr(file, '\\') ? strrchr(file, '\\') + 1 : file;

    va_list list;

    va_start(list, format);
    auto len = vsnprintf(nullptr, 0, format, list) + 1;
    va_end(list);

    auto buffer = std::make_unique<char[]>(len);
    auto message = buffer.get();

    va_start(list, format);
    vsnprintf(message, len, format, list);
    va_end(list);

    (*w) << file << ":" << line << ":" << function << " " << _prefix << " " << message << std::endl;
}

void unset_writer() noexcept
{
    delete w;
    w = nullptr;
}

}; // namespace logger