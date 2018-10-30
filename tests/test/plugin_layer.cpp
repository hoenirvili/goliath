#include "plugin_layer.h"
#include "cfgtrace/api/types.h"
#include <algorithm>

static size_t max_layer(const layer_informations &infos)
{
    auto max_element = std::max_element(
      infos.begin(), infos.end(),
      [](const layer_information &a, const layer_information &b) {
          return (a.first < b.first);
      });
    return max_element->first;
}

plugin_layer::plugin_layer(const layer_informations &&infos)
{
    // find how many layers we need to create
    this->n = max_layer(infos);

    // create an array of n + 1 layers
    // because the highest layer will be at
    // this->p[this->n]
    this->p = new PluginLayer *[n + 1];

    // create a new node in the linked list for
    // every layer
    for (size_t i = 0; i < n + 1; i++)
        p[i] = new PluginLayer{0};

    for (const auto &info : infos) {
        // touch only the layers that we need to fill
        p[info.first]->data = new PluginReport();
        p[info.first]->data->plugin_name = info.second;
    }
}

plugin_layer::~plugin_layer()
{
    if (!this->n || !this->p)
        return;

    for (size_t i = 0; i < n; i++) {
        if (!p[i]->data) // not all layers are filled
            continue;
        delete p[i]->data;
    }

    for (size_t i = 0; i < n; i++)
        delete p[i];

    delete[] p;
    this->p = nullptr;
    this->n = 0;
}

PluginLayer **plugin_layer::get() const noexcept
{
    return this->p;
}