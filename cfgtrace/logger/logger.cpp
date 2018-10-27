#include "cfgtrace/logger/logger.h"
#include <cstdarg>
#include <fstream>
#include <functional>
#include <map>
#include <ostream>
#include <string>

namespace logger
{
static creator fn;

static std::ostream *file_creator(const char *name)
{
    std::ostream *file;
    if (file = new std::fstream(name, std::ios::app); (!*(file))) {
        delete file;
        file = nullptr;
    }

    return file;
}

static std::ostream *out;

void write(level l, const char *file, const int line, const char *function, const char *format, ...) noexcept
{
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

    (*out) << file << ":" << line << ":" << function << " " << _prefix << " " << message << '\n';
}

void custom_creation(creator create) noexcept
{
    fn = create;
}

bool initialise(const char *name)
{
    if (out)
        return true;

    if (!fn)
        fn = file_creator;

    if (out = fn(name); !out)
        return false;

    return true;
}

void clean() noexcept
{
    if (out) {
        delete out;
        out = nullptr;
    }
}

}; // namespace logger