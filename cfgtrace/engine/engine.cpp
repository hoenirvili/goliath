#include "cfgtrace/engine/engine.h"
#include "cfgtrace/api/types.h"
#include "cfgtrace/error/error.h"
#include "cfgtrace/error/win32.h"
#include <cstdint>
#include <stdexcept>
#include <windows.h>

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
    this->memory = (std::byte *)MapViewOfFile(file_mapping, FILE_MAP_ALL_ACCESS, 0, 0, engine::BUFFER_SIZE);
    if (this->memory == nullptr)
        throw ex(error::win32, "cannot open a view into the address space of a calling process");
}

engine &engine::operator=(engine &&other)
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

std::byte *engine::context() const noexcept
{
    return &this->memory[engine::CONTEXT_OFFSET];
}

size_t *engine::flags() const
{
    void *start = &this->memory[engine::FLAGS_OFFSET];
    size_t space = sizeof(size_t);
    if (!std::align(alignof(size_t), sizeof(size_t), start, space))
        throw ex(std::runtime_error, "cannot get addr to flags, addr is not aligned");

    return new (start) size_t;
}

PluginReport **engine::plugin_report() const
{
    void *start = &this->memory[engine::PLUGINS_REPORT_SIZE_OFFSET];
    size_t space = sizeof(PluginReport *);
    if (!std::align(alignof(PluginReport **), sizeof(PluginReport *), start, space))
        throw ex(std::runtime_error, "cannot get addr to addr to plugin report, addr is not aligned");

    return new (start) PluginReport *;
}

size_t engine::plugin_report_size() const
{
    void *start = &this->memory[engine::PLUGINS_REPORT_SIZE_OFFSET];
    size_t space = sizeof(size_t);
    if (!std::align(alignof(size_t), sizeof(size_t), start, space))
        throw ex(std::runtime_error, "cannot get addr to report size, addr is not aligned");

    size_t *size = new (start) size_t;
    return *size;
}

size_t engine::process_stacktop() const
{
    void *start = &this->memory[engine::PROCESS_STACKTOP_OFFSET];
    size_t space = sizeof(size_t);
    if (!std::align(alignof(size_t), sizeof(size_t), start, space))
        throw ex(std::runtime_error, "cannot get process stacktop value, addr is not aligned");

    size_t *size = new (start) size_t;
    return *size;
}

char *engine::thread_info() const noexcept
{
    return reinterpret_cast<char *>(&this->memory[engine::THREAD_INFO_OFFSET]);
}

char *engine::protected_pids() const noexcept
{
    return reinterpret_cast<char *>(&this->memory[engine::PROTECTED_PIDS_OFFSET]);
}

char *engine::protected_files() const noexcept
{
    return reinterpret_cast<char *>(&this->memory[engine::PROTECTED_FILES_OFFSET]);
}

std::byte *engine::cfg_memory_region() const noexcept
{
    return &this->memory[engine::SHARED_CFG];
}

size_t engine::cfg_memory_region_size() const noexcept
{
    return engine::BUFFER_SIZE - engine::SHARED_CFG;
}

size_t *engine::cfg_iteration() const
{
    void *start = this->cfg_memory_region();
    size_t space = sizeof(size_t);
    if (!std::align(alignof(size_t), sizeof(size_t), start, space))
        throw ex(std::runtime_error, "cannot get iteration value, addr is not aligned");

    return new (start) size_t;
}

size_t *engine::cfg_size() const
{
    std::byte *base = this->cfg_memory_region();
    size_t offset = sizeof(*this->cfg_iteration());
    void *start = &base[offset];
    size_t space = sizeof(size_t);
    if (!std::align(alignof(size_t), sizeof(size_t), start, space))
        throw ex(std::runtime_error, "cannot get the size of cfg, addr is not aligned");

    return new (start) size_t;
}

std::byte *engine::cfg_serialize_memory_region() const
{
    std::byte *base = this->cfg_memory_region();
    size_t it_offset = sizeof(*this->cfg_iteration());
    size_t size_offset = sizeof(*this->cfg_size());
    return &base[it_offset + size_offset];
}

static engine *e;

static creator fn;

static engine *default_creator(HANDLE file_mapping)
{
    return new engine(file_mapping);
}

void custom_creation(creator create)
{
    fn = create;
}

bool is_initialised() noexcept
{
    return (e);
}

engine *instance()
{
    if (!e) {
        HANDLE file_mapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, memsharedname);
        if (!file_mapping)
            return nullptr;

        if (!fn)
            fn = default_creator;

        return fn(file_mapping);
    }

    return e;
}

void clean() noexcept
{
    if (!e)
        return;

    delete e;
    e = nullptr;
}

}; // namespace engine