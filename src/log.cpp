#include "log.hpp"

Log* Log::instanta = 0;

Log::Log()
{
    fname = "";
}

Log::~Log()
{
    fname = "";
    delete instanta;
}

void Log::DestroyInstance()
{
    delete instanta;
}

Log* Log::GetInstance()
{
    if (!instanta)
        instanta = new Log();
    return instanta;
}

void Log::DebugMsg(const char* format, ...)
{
    if (!fname.empty()) {
        char dest[1024];
        va_list argptr;
        va_start(argptr, format);
        vsprintf(dest, format, argptr);
        va_end(argptr);
        fstream fout;
        fout.open(fname, fstream::out | fstream::app);
        fout << dest;
        fout.close();
    }
}

void Log::ChangeLogName(const char* log)
{
    char temp[MAX_PATH];
    GetFullPathName(log, MAX_PATH, temp, NULL);
    fname = temp;
}

void Log::RemovePrevious()
{
    if (!fname.empty())
        DeleteFile(fname.c_str());
}
