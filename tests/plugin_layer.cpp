#include "plugin_layer.h"
#include "cfgtrace/api/types.h"

plugin_layer::plugin_layer(size_t layer, const char *plugin_name)
{
    this->layer = layer;
    this->p = new PluginLayer *[this->layer + 1];
    // create every layer from the base addr
    // to p[this->layer];
    for (size_t i = 0; i <= this->layer; i++) {
        p[i] = new PluginLayer();
        p[i]->data = nullptr;
        p[i]->nextnode = nullptr;
    }

    if (plugin_name) {
        p[this->layer]->data = new PluginReport();
        p[this->layer]->data->plugin_name = plugin_name;
    }
}

plugin_layer::~plugin_layer()
{
    if (!this->p)
        return;

    for (size_t i = 0; i <= this->layer; i++) {
        if (p[i] != nullptr)
            continue;

        if (p[i]->data)
            delete p[i]->data;
        delete p[i];
    }
    delete[] p;
}

PluginLayer **plugin_layer::get() const noexcept
{
    return this->p;
}