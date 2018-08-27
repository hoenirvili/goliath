#include "cfgtrace/engine/engine.h"
#include "cfgtrace/engine/types.h"
#include "cfgtrace/error/win32.h"
#include <cstdint>
#include <stdexcept>
#include <windows.h>

using namespace std;

namespace engine
{
engine::engine(HANDLE file_mapping)
{
    // maps a view of a file mapping into the address space
    // of a calling process
    this->memory = (BYTE *)MapViewOfFile(
      file_mapping, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
    if (this->memory == nullptr)
        throw error::win32(
          "cannot open a view into the address space of a calling process");
}

engine& engine::operator=(engine other)
{
	this->memory = other.memory;
    return *this;
}
	
engine::~engine()
{
}

PluginLayer *
engine::plugin_interface(char *pluginname, size_t layer, PluginLayer **layers)
{
    PluginLayer *scanner = layers[layer];

    while (scanner) {
        if (scanner->data && scanner->data->plugin_name &&
            !strcmp(scanner->data->plugin_name, pluginname))
            return scanner;

        scanner = scanner->nextnode;
    }

    return nullptr;
}

char *engine::log_name() const noexcept
{
    return reinterpret_cast<char *>(&this->memory[this->LOGNAME_OFFSET]);
}

char *engine::plugin_path() const noexcept
{
    return reinterpret_cast<char *>(&this->memory[this->PLUGINS_OFFSET]);
}

uint8_t *engine::context() const noexcept
{
    return &this->memory[this->CONTEXT_OFFSET];
}

size_t *engine::flags() const
{
    uint8_t *mem = &this->memory[this->FLAGS_OFFSET];
    if (is_aligned<size_t>(mem))
        throw runtime_error("cannot get addr to flags, addr is not aligned");

    return reinterpret_cast<size_t *>(mem);
}

PluginReport **engine::plugin_report() const
{
    uint8_t *mem = &this->memory[this->PLUGINS_REPORT_SIZE_OFFSET];
    if (!is_aligned<PluginReport *>(mem))
        throw runtime_error(
          "cannot get addr to addr to plugin report, addr is not aligned");

    return reinterpret_cast<PluginReport **>(mem);
}

size_t engine::plugin_report_size() const
{
    uint8_t *mem = &this->memory[this->PLUGINS_REPORT_SIZE_OFFSET];
    if (!is_aligned<size_t>(mem))
        throw runtime_error(
          "cannot get addr to report size, addr is not aligned");

    return *reinterpret_cast<size_t *>(mem);
}

size_t engine::process_stacktop() const
{
    uint8_t *mem = &this->memory[PROCESS_STACKTOP_OFFSET];
    if (is_aligned<size_t>(mem))
        throw runtime_error(
          "cannot get process stacktop value, addr is not aligned");

    return *reinterpret_cast<size_t *>(mem);
}

uint8_t *engine::cfg_memory_region() const noexcept
{
    return &this->memory[this->SHARED_CFG];
}

size_t engine::cfg_memory_region_size() const noexcept
{
    return this->BUFFER_SIZE - this->SHARED_CFG;
}

int *engine::cfg_iteration() const
{
    uint8_t *mem = this->cfg_memory_region();
    if (!is_aligned<int>(mem))
        throw runtime_error("cannot get iteration value, addr is not aligned");

    return reinterpret_cast<int *>(mem);
}

int *engine::cfg_size() const
{
    uint8_t offset = sizeof(*this->cfg_iteration());
    uint8_t *base = this->cfg_memory_region();
    return reinterpret_cast<int *>(&base[offset]);
}

uint8_t *engine::cfg_serialize_memory_region() const
{
    uint8_t *base = this->cfg_memory_region();
    size_t it_offset = sizeof(*this->cfg_iteration());
    size_t size_offset = sizeof(*this->cfg_size());
    return &base[it_offset + size_offset];
}

}; // namespace engine