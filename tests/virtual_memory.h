#pragma once

#include "cfgtrace/engine/engine.h"
#include <windows.h>

class virtual_memory
{
public:
    HANDLE handler;
    unsigned char *file_view;

    virtual_memory();
    ~virtual_memory();
    const size_t size = engine::engine::BUFFER_SIZE;
    void enable_log_name();
    size_t interation_count();
    void set_iteration_count(size_t n);
};