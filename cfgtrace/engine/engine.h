#pragma once

#include "cfgtrace/api/types.h"
#include <cstdint>
#include <windows.h>

namespace engine
{
class engine
{
private:
    const int LOGNAME_OFFSET = 0x0000;
    const int PLUGINS_OFFSET = 0x1000;
    const int CONTEXT_OFFSET = 0x2000;
    const int FLAGS_OFFSET = 0x3000;
    const int PLUGINS_REPORT_OFFSET = 0x4000;
    const int PLUGINS_REPORT_SIZE_OFFSET = 0x5000;
    const int PROCESS_STACKTOP_OFFSET = 0xC000;
    const int SHARED_CFG = 0x40000;
    const int BUFFER_SIZE = 0x100000;
    uint8_t *memory;

public:
    engine(HANDLE file_mapping);
    engine() : memory(nullptr) {}
    engine &operator=(engine other);
    ~engine() = default;
    PluginLayer *plugin_interface(char *pluginname, size_t layer, PluginLayer **layers) const noexcept;
    char *log_name() const noexcept;
    char *plugin_path() const noexcept;
    uint8_t *context() const noexcept;
    size_t *flags() const;
    PluginReport **plugin_report() const;
    size_t plugin_report_size() const;
    size_t process_stacktop() const;
    uint8_t *cfg_memory_region() const noexcept;
    size_t cfg_memory_region_size() const noexcept;
    size_t *cfg_iteration() const;
    size_t *cfg_size() const;
    uint8_t *cfg_serialize_memory_region() const;
};
}; // namespace engine