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
    size_t size = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS, 
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

static void ReadFromPipe(HANDLE reader)
{
    DWORD nread, nwrite;
    CHAR buffer[255] = {0};
    BOOL success = FALSE;
    HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    for (;;) {
        success = ReadFile(reader, buffer, 255, &nread, NULL);
        if (!success || nread == 0)
            break;

        success = WriteFile(hParentStdOut, buffer, nread, &nwrite, NULL);
        if (!success)
            break;
    }
}

int execute_command(const string& command)
{
    SECURITY_ATTRIBUTES security_attr;
    security_attr.nLength = sizeof(security_attr);
    security_attr.bInheritHandle = TRUE;
    security_attr.lpSecurityDescriptor = NULL;

    HANDLE out_read, out_write;
    if (!CreatePipe(&out_read, &out_write, &security_attr, 0))
        return 1;

    if (!SetHandleInformation(out_read, HANDLE_FLAG_INHERIT, 0))
        return 1;

    string ret = "";
    STARTUPINFO startup_information = {0};
    PROCESS_INFORMATION process_information = {0};
    startup_information.dwFlags = STARTF_USESTDHANDLES;
    startup_information.cb = sizeof(PROCESS_INFORMATION);
    startup_information.hStdError = out_write;
    TCHAR commnad[MAX_PATH] = {0};
    _stprintf_s(commnad, _T("%s"), command.c_str());
    if (!CreateProcess(
		NULL, 
		commnad, 
		NULL, 
		NULL, 
		FALSE, 
		NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, 
		NULL, 
		NULL, 
		&startup_information, 
		&process_information
	)) goto _exit;
	//TODO(hoenir): fix this
	ReadFromPipe(startup_information.hStdError);
    WaitForSingleObject(process_information.hProcess, INFINITE);

_exit:
    CloseHandle(startup_information.hStdError);
    CloseHandle(process_information.hProcess);
    CloseHandle(process_information.hThread);

    return 0;
}