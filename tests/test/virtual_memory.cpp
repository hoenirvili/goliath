#include "virtual_memory.h"
#include "cfgtrace/engine/engine.h"
#include <memory>
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
    if (this->file_view) {
        UnmapViewOfFile(this->file_view);
        this->file_view = nullptr;
    }

    CloseHandle(this->handler);
}

const char *virtual_memory::logger_name() const noexcept
{
    return this->file_log_name;
}

void virtual_memory::enable_log_name()
{
    if (!this->file_view)
        throw std::runtime_error("file_view has not been mapped");
    /**
     * write the first bytes the test log file name
     */
    size_t file_log_size = strlen(this->file_log_name);
    memcpy(this->file_view, this->file_log_name, file_log_size);
}

void virtual_memory::set_iteration_count(size_t n)
{
    void *start = &this->file_view[engine::engine::SHARED_CFG];
    memcpy(start, &n, sizeof(n));
};

size_t virtual_memory::iteration_count()
{
    void *start = &this->file_view[engine::engine::SHARED_CFG];
    size_t space = sizeof(size_t);
    if (!std::align(alignof(size_t), sizeof(size_t), start, space))
        throw std::runtime_error("ptr is not aligned");

    auto it = new (start) size_t;
    return *it;
}