#include "cfgtrace/error/win32.h"
#include "cfgtrace/format/win32.h"
#include <stdexcept>
#include <windows.h>

using namespace std;

namespace error
{
string win32::context() const noexcept
{
    DWORD id = GetLastError();
    if (id == 0)
        return "";

    auto last_string = format::win32_error(id);
    if (last_string.empty())
        return "";

    return last_string + ":";
}

const char *win32::what() const
{
    return runtime_error::what();
}

}; // namespace error




