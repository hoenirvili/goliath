#pragma once

#include "cfgtrace/api/types.h"

class plugin_layer
{
public:
    plugin_layer(size_t layer, const char *plugin_name);
    ~plugin_layer();
    PluginLayer **get() const noexcept;

private:
    size_t layer = 0;
    PluginLayer **p = nullptr;
};
