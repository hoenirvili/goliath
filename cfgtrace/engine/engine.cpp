#include "cfgtrace/engine/engine.h"
#include "cfgtrace/api/types.h"
#include "cfgtrace/error/error.h"
#include "cfgtrace/error/win32.h"
#include <cstdint>
#include <stdexcept>
#include <windows.h>

using namespace std;

namespace engine
{
/*
    memory layout x86 - 32bit
    int 4 bytes => number of iterations
    int 4 bytes => the size of the hole control flow graph in memory
    control_flow_graph  => the hole control_flow_graph in memory
*/

engine::engine(HANDLE file_mapping)
{
    // maps a view of a file mapping into the address space
    // of a calling process
    this->memory = (BYTE *)MapViewOfFile(file_mapping, FILE_MAP_ALL_ACCESS, 0, 0, engine::BUFFER_SIZE);
    if (this->memory == nullptr)
        throw ex(error::win32, "cannot open a view into the address space of a calling process");
}

engine &engine::operator=(engine &other)
{
    this->memory = other.memory;
    other.memory = nullptr;
    return *this;
}

engine::~engine()
{
    if (this->memory)
        UnmapViewOfFile(this->memory);
}

PluginLayer *engine::plugin_interface(char *pluginname, size_t layer, PluginLayer **layers) const noexcept
{
    PluginLayer *scanner = layers[layer];

    while (scanner) {
        if (scanner->data && scanner->data->plugin_name && !strcmp(scanner->data->plugin_name, pluginname))
            return scanner;

        scanner = scanner->nextnode;
    }

    return nullptr;
}

char *engine::log_name() const noexcept
{
    return reinterpret_cast<char *>(&this->memory[engine::LOGNAME_OFFSET]);
}

char *engine::plugin_path() const noexcept
{
    return reinterpret_cast<char *>(&this->memory[engine::PLUGINS_OFFSET]);
}

uint8_t *engine::context() const noexcept
{
    return &this->memory[engine::CONTEXT_OFFSET];
}

size_t *engine::flags() const
{
    uint8_t *mem = &this->memory[engine::FLAGS_OFFSET];
    if (is_aligned<size_t>(mem))
        throw ex(runtime_error, "cannot get addr to flags, addr is not aligned");

    return reinterpret_cast<size_t *>(mem);
}

PluginReport **engine::plugin_report() const
{
    uint8_t *mem = &this->memory[engine::PLUGINS_REPORT_SIZE_OFFSET];
    if (!is_aligned<PluginReport *>(mem))
        throw ex(runtime_error, "cannot get addr to addr to plugin report, addr is not aligned");

    return reinterpret_cast<PluginReport **>(mem);
}

size_t engine::plugin_report_size() const
{
    uint8_t *mem = &this->memory[engine::PLUGINS_REPORT_SIZE_OFFSET];
    if (!is_aligned<size_t>(mem))
        throw ex(runtime_error, "cannot get addr to report size, addr is not aligned");

    return *reinterpret_cast<size_t *>(mem);
}

size_t engine::process_stacktop() const
{
    uint8_t *mem = &this->memory[engine::PROCESS_STACKTOP_OFFSET];
    if (is_aligned<size_t>(mem))
        throw ex(runtime_error, "cannot get process stacktop value, addr is not aligned");

    return *reinterpret_cast<size_t *>(mem);
}

uint8_t *engine::cfg_memory_region() const noexcept
{
    return &this->memory[engine::SHARED_CFG];
}

size_t engine::cfg_memory_region_size() const noexcept
{
    return engine::BUFFER_SIZE - engine::SHARED_CFG;
}

size_t *engine::cfg_iteration() const
{
    uint8_t *mem = this->cfg_memory_region();
    if (!is_aligned<size_t>(mem))
        throw ex(runtime_error, "cannot get iteration value, addr is not aligned");

    return reinterpret_cast<size_t *>(mem);
}

size_t *engine::cfg_size() const
{
    uint8_t *base = this->cfg_memory_region();
    uint8_t offset = sizeof(*this->cfg_iteration());
    return reinterpret_cast<size_t *>(&base[offset]);
}

uint8_t *engine::cfg_serialize_memory_region() const
{
    uint8_t *base = this->cfg_memory_region();
    size_t it_offset = sizeof(*this->cfg_iteration());
    size_t size_offset = sizeof(*this->cfg_size());
    return &base[it_offset + size_offset];
}

}; // namespace engine