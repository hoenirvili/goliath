#pragma once

#include "cfgtrace/api/types.h"
#include <cstddef>
#include <functional>
#include <windows.h>

namespace engine
{
/**
 * engine type can interact with the engine shared memory page
 * and retrive information used in the public facing API
 * that the plugin it expose
 */
class engine
{
private:
    std::byte *memory = nullptr;

public:
    /**
     * list of offset addresses that the engine
     * writes differrent information to
     */
    static constexpr int LOGNAME_OFFSET = 0x0000;
    static constexpr int PLUGINS_OFFSET = 0x1000;
    static constexpr int CONTEXT_OFFSET = 0x2000;
    static constexpr int FLAGS_OFFSET = 0x3000;
    static constexpr int PLUGINS_REPORT_OFFSET = 0x4000;
    static constexpr int PLUGINS_REPORT_SIZE_OFFSET = 0x5000;
    static constexpr int THREAD_INFO_OFFSET = 0x20000;
    static constexpr int PROCESS_STACKTOP_OFFSET = 0xC000;
    static constexpr int PROTECTED_PIDS_OFFSET = 0x28000;
    static constexpr int PROTECTED_FILES_OFFSET = 0x29000;
    static constexpr int SHARED_CFG = 0x40000;
    static constexpr int BUFFER_SIZE = 0x100000;

    /**
     * return a new engine instance based on the handle provided
     */
    engine(HANDLE file_mapping);
    engine() = default;
    ~engine();

    /**
     * plugin_interface returns the node of an plugin
     * specified in by the pluginname inside layers list
     */
    PluginLayer *
    plugin_interface(char *pluginname, size_t layer, PluginLayer **layers) const
      noexcept;

    /**
     * log_name return the main log name that the engine logs to
     */
    char *log_name() const noexcept;

    /**
     * plugin_path returns the plugin_path directory
     */
    char *plugin_path() const noexcept;

    std::byte *context() const noexcept;
    size_t *flags() const;
    PluginReport **plugin_report() const;
    size_t plugin_report_size() const;
    size_t process_stacktop() const;
    char *thread_info() const noexcept;
    char *protected_pids() const noexcept;
    char *protected_files() const noexcept;
    std::byte *cfg_memory_region() const noexcept;
    size_t cfg_memory_region_size() const noexcept;
    size_t *cfg_iteration() const;
    size_t *cfg_size() const;
    std::byte *cfg_serialize_memory_region() const;
}; // class engine

bool is_initialised() noexcept;

engine *instance();

void clean() noexcept;

using creator = std::function<engine *(HANDLE file_mapping)>;

void custom_creation(creator create);

}; // namespace engine