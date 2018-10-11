#pragma once

#include "cfgtrace/engine/engine.h"
#include <windows.h>

class virtual_memory
{
public:
    virtual_memory();
    ~virtual_memory();
    HANDLE handler;
    unsigned char *file_view;
    const size_t size = engine::engine::BUFFER_SIZE;
    void enable_log_name();
};