#include "cfgtrace/logger/logger.h"
#include <cstdarg>
#include <fstream>
#include <map>
#include <memory>
#include <string>

using namespace std;

namespace logger
{
static ostream *out = nullptr;

void init(ostream *os) noexcept
{
    if (out)
        return;

    out = os;
}

void write(level l, const char *file, const int line, const char *function, const char *format, ...)
{
    const map<level, string> table = {{level::error, "ERROR"}, {level::warning, "WARNING"}, {level::info, "INFO"}};

    auto _prefix = "|" + table.at(l) + "|";
    file = strrchr(file, '\\') ? strrchr(file, '\\') + 1 : file;
    va_list list;
    va_start(list, format);
    auto len = vsnprintf(nullptr, 0, format, list) + 1;
    auto message = make_unique<char[]>(len);
    vsnprintf(message.get(), len, format, list);
    va_end(list);
    (*out) << file << ":" << line << ":" << function << " " << _prefix << " " << message.get() << std::endl;
}

void clean() noexcept
{
    if (!out)
        return;
    delete out;
    out = nullptr;
}

}; // namespace logger