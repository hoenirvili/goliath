#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <windows.h>
using namespace std;

#pragma warning(disable : 4996)

class Log {
private:
    Log();
    ~Log();

    static Log* instanta;

    string fname;

public:
    static Log* GetInstance();
    void DestroyInstance();
    void DebugMsg(const char* format, ...);
    void ChangeLogName(const char* log);
    void RemovePrevious();
};
