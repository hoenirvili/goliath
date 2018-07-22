#include "exec.h"
#include "windows.h"
#include <string>
#include <tchar.h>

using namespace std;

static string LastErrorAsString(DWORD id)
{
    if (id == 0)
        return "";

    LPSTR buffer = nullptr;
    auto flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS;

	size_t size = FormatMessageA(
		flags, 
		NULL, 
		id, 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
		(LPSTR) &buffer, 
		0, 
		NULL
	);

    string error(buffer, size);
    LocalFree(buffer);
    return error;
}

string execute_command(const string& command)
{
    SECURITY_ATTRIBUTES sa = {
        sizeof(SECURITY_ATTRIBUTES),// nLength
        nullptr,					// lpSecurityDescriptor
        true						// bInheritHandle
    };

    HANDLE stderr_reader, stderr_writer;

    if (!CreatePipe(&stderr_reader, &stderr_writer, &sa, 0))
        return LastErrorAsString(GetLastError());

    if (!SetHandleInformation(stderr_reader, HANDLE_FLAG_INHERIT, 0))
        return LastErrorAsString(GetLastError());

    PROCESS_INFORMATION pi = {0};
    STARTUPINFO si = {0};
    bool success = false;

    si.cb = sizeof(si);
    si.hStdError = stderr_writer;
    si.dwFlags = STARTF_USESTDHANDLES;

    success = CreateProcessA(
		NULL,					// name
        (LPSTR) command.c_str(),// command line
        NULL,					// process security attributes
        NULL,					// primary thread security attributes
        TRUE,					// handles are inherited
        0,						// creation flags
        NULL,					// use parent's environment
        NULL,					// use parent's current directory
        &si,					// STARTUPINFO pointer
        &pi);					// receives PROCESS_INFORMATION

    CloseHandle(stderr_writer); // the parrent does not need the writer

    string stderr_output = "";

    if (!success)
        goto _cleaup;

    for (success = false; !success;) {
        success = WaitForSingleObject(pi.hProcess, 50) == WAIT_OBJECT_0;
        for (;;) {
            char buffer[0xff] = {0};
            DWORD n;
            if (!PeekNamedPipe(stderr_reader, NULL, 0, NULL, &n, NULL) || !n)
                break;
            if (!ReadFile(stderr_reader, buffer, 0xff, &n, NULL) || !n)
                break;
            stderr_output += string(buffer, n);
        }
    }

_cleaup:
    CloseHandle(stderr_reader);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return stderr_output;
}