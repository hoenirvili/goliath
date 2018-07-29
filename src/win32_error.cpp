#include <stdexcept>
#include <string>
#include <windows.h>
#include "format.h"
#include "win32_error.h"

using namespace std;

string win32_error::context() const noexcept
{
    DWORD id = GetLastError();
    if (id == 0)
        return "";

    auto last_string = fmt_win32_error(id);
    if (last_string.empty())
        return "";

    return last_string + ":";
}

const char* win32_error::what() const
{
    return runtime_error::what();
}