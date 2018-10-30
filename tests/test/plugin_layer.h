#pragma once

#include "cfgtrace/api/types.h"
#include <utility>
#include <vector>

using layer_information = std::pair<size_t, const char *>;

using layer_informations = std::vector<layer_information>;

class plugin_layer
{
public:
    plugin_layer(const layer_informations &&infos);
    ~plugin_layer();
    PluginLayer **get() const noexcept;

private:
    size_t n = 0;
    PluginLayer **p = nullptr;
};
