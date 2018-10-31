#pragma once

#include "cfgtrace/api/types.h"
#include <tuple>
#include <vector>

using layer_information = std::tuple<size_t, const char *, void *, void *>;
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

    bool find_in(std::vector<size_t> arr, size_t item) const noexcept;
    void plugin_layer::append_into_linked_list(PluginLayer *layer,
                                               size_t target_layer,
                                               const layer_informations &infos,
                                               size_t at) const noexcept;
};
