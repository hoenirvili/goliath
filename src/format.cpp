#include "format.h"

using namespace std;

string fmt_win32_error(DWORD id)
{
    if (id == 0)
        return "";

    LPSTR buffer = nullptr;

    auto flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | // allocate the buffer for me
                 FORMAT_MESSAGE_FROM_SYSTEM | // retrive err system message
                 FORMAT_MESSAGE_IGNORE_INSERTS | //  indicate that the message number you passed is an error code
                 FORMAT_MESSAGE_MAX_WIDTH_MASK; // do not append \r\n

    size_t size = FormatMessageA( flags, NULL, id, 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
		(LPSTR)(&buffer), 0, NULL);
    if (size == 0)
        throw system_error(id, system_category(), "FormatMessageA failed");
    if (buffer == nullptr)
        throw system_error(id, system_category(), "cannot unallocated buffer");

    // trim space and .
    while (buffer[size - 1] == '.' || 
		buffer[size - 1] == ' ')
        size--;

    string error(buffer, size);
    if (LocalFree(buffer))
        throw system_error(id, system_category(), "LocalFree failed");

    return error;
}