#include "virtual_memory.h"
#include "cfgtrace/engine/engine.h"
#include <stdexcept>
#include <windows.h>

virtual_memory::virtual_memory()
{
    /**
     * CreateFileMappingA
     * creates a file mapping object of a specified size that is backed by
     * the system paging file instead of by a file in the file system
     */
    this->handler = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, this->size, memsharedname);
    if (!this->handler)
        throw std::runtime_error("CreateFileMappingA");

    /**
     * in oder to write to that memory region we need
     * to first create a view into that handler
     */
    LPVOID view = MapViewOfFile(this->handler, FILE_MAP_ALL_ACCESS, 0, 0, this->size);
    if (!view) {
        CloseHandle(this->handler);
        throw std::runtime_error("MapViewOfFile");
    }
    this->file_view = static_cast<unsigned char *>(view);
}

virtual_memory::~virtual_memory()
{
    if (this->file_view)
        UnmapViewOfFile(this->file_view);

    CloseHandle(this->handler);
}

void virtual_memory::enable_log_name()
{
    if (!this->file_view)
        throw std::runtime_error("file_view has not been mapped");
    /**
     * write the first bytes the test log file name
     */
    const char *file_log = "test_log_file.log";
    size_t file_log_size = strlen(file_log);
    memcpy(this->file_view, file_log, file_log_size);
    /**
     * we are done writing to that memory region
     * make room for other processes to view in there
     */
    UnmapViewOfFile(this->file_view);
    this->file_view = nullptr;
}