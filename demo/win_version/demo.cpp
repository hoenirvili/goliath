#include <Windows.h>
#include <VersionHelpers.h>

DWORD get_version(void)
{
    if (!IsWindows8OrGreater())
        return 0x2312;

    return 0x0;
}

int main(void)
{
    DWORD limit = GdiGetBatchLimit();
    bool limit_above = false;
    if (limit > 0xfda1)
        limit_above = true;

    DWORD version = 0x0;
    if (limit_above)
        DWORD version = get_version();

    WORD decompile = 0x10;
    if (version > 6)
        if (!limit_above)
            decompile--;

    WORD loop = 0x0;
    for (WORD i = 0; decompile > 0; --decompile) {
        version = 0;
        if (decompile % 2 == 0)
            version = get_version();

        if (version == 0)
            loop = 1;
    }

    return loop;
}