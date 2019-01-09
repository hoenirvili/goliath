#pragma once

#include <goliath/api/types.h>

#include <tuple>
#include <vector>

using layer_information = std::tuple<size_t, const char *, void *, void *>;
using layer_informations = std::vector<layer_information>;

struct plugin_layer {
    PluginLayer **get() const noexcept;

    plugin_layer() = default;                       // default empty ctor
    plugin_layer(const layer_informations &&infos); // params ctor
    ~plugin_layer();                                // destructor

    plugin_layer(const plugin_layer &);            // copy ctor
    plugin_layer &operator=(const plugin_layer &); // copy assign ctor

    plugin_layer(plugin_layer &&);            // move ctor
    plugin_layer &operator=(plugin_layer &&); // move assign ctor

private:
    size_t n = 0;
    PluginLayer **p = nullptr;

    bool find_in(std::vector<size_t> arr, size_t item) const noexcept;
    void plugin_layer::append_into_linked_list(PluginLayer *layer,
                                               size_t target_layer,
                                               const layer_informations &infos,
                                               size_t at) const noexcept;
};
