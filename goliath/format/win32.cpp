#include "goliath/format/win32.h"
#include <stdexcept>
#include <windows.h>

using namespace std;

namespace format
{
string win32_error(DWORD id)
{
    if (id == 0)
        return "";

    LPSTR buffer = nullptr;

    auto flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                 FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK;

    size_t size = FormatMessageA(flags, nullptr, id,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 (LPSTR)(&buffer), 0, nullptr);
    if (size == 0)
        throw system_error(id, system_category(), "FormatMessageA failed");
    if (buffer == nullptr)
        throw system_error(id, system_category(), "cannot unallocated buffer");

    // trim space and .
    while (buffer[size - 1] == '.' || buffer[size - 1] == ' ')
        size--;

    string error(buffer, size);
    if (LocalFree(buffer))
        throw system_error(id, system_category(), "LocalFree failed");

    return error;
}

}; // namespace format
