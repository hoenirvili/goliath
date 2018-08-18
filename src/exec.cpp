#include "exec.h"
#include "win32_error.h"
#include "format.h"
#include <windows.h>

using namespace std;

void execute_command(const string& command, string* process_stderr, string* process_exit)
{
    SECURITY_ATTRIBUTES sa = {
        sizeof(SECURITY_ATTRIBUTES), // nLength
        nullptr, // lpSecurityDescriptor
        true // bInheritHandle
    };

    HANDLE stderr_reader, stderr_writer;
    DWORD code = 0;

    if (!CreatePipe(&stderr_reader, &stderr_writer, &sa, 0))
        throw win32_error("CreatePipe failed");

    if (!SetHandleInformation(stderr_reader, HANDLE_FLAG_INHERIT, 0))
        throw win32_error("SetHandleInformation failed");

    PROCESS_INFORMATION pi = {0};
    STARTUPINFO si = {0};
    bool success = false;

    si.cb = sizeof(si);
    si.hStdError = stderr_writer;
    si.hStdOutput = INVALID_HANDLE_VALUE;
    si.hStdInput = INVALID_HANDLE_VALUE;
    si.dwFlags = STARTF_USESTDHANDLES;

    success = CreateProcessA(NULL, // name
        (LPSTR) command.c_str(), // command line
        NULL, // process security attributes
        NULL, // primary thread security attributes
        TRUE, // handles are inherited
        0, // creation flags
        NULL, // use parent's environment
        NULL, // use parent's current directory
        &si, // STARTUPINFO pointer
        &pi // receives PROCESS_INFORMATION
    );

    // the parrent does not need the writer
    if (!CloseHandle(stderr_writer))
        throw win32_error("cannot close the stderr writer parrent handler");

    if (!success)
        throw win32_error("CreateProcessA failed");

    if (process_stderr) {
        string stderr_output = "";
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
        
		if (!stderr_output.empty())
			stderr_output.erase(stderr_output.size() - 2, 2);
        
		*process_stderr = stderr_output;
    }

    if (process_exit) {
        if (!GetExitCodeProcess(pi.hProcess, &code))
            throw win32_error("cannot get exit code");
        if (code)
            *process_exit = fmt_win32_error(code);
    }

    if (!CloseHandle(pi.hProcess))
        throw win32_error("cannot close process handler");
    if (!CloseHandle(pi.hThread))
        throw win32_error("cannot close thread handler");

    if (!CloseHandle(stderr_reader))
        throw win32_error("cannot close stderr handler");
}