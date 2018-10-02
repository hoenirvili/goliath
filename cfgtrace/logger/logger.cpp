#include "cfgtrace/logger/logger.h"
#include <cstdarg>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>

using namespace std;

namespace logger
{
void init(ostream *os) noexcept
{
    unique_ptr<ostream> out;
    out.reset(os);
}

void write(level l, const char *file, const int line, const char *function, const char *format, ...)
{
    return;
    // if (!out)
    //    return;

    static const unordered_map<level, string> table = {
      {level::error, "ERROR"}, {level::warning, "WARNING"}, {level::info, "INFO"}};

    auto _prefix = "|" + table.at(l) + "|";
    file = strrchr(file, '\\') ? strrchr(file, '\\') + 1 : file;
    va_list list;
    va_start(list, format);
    auto len = vsnprintf(nullptr, 0, format, list) + 1;
    auto message = make_unique<char[]>(len);
    vsnprintf(message.get(), len, format, list);
    va_end(list);
    //(*out) << file << ":" << line << ":" << function << " " << _prefix << " " << message.get() << std::endl;
}

}; // namespace logger
