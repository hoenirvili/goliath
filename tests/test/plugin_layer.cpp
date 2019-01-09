#include "plugin_layer.h"

#include <algorithm>
#include <cassert>
#include <goliath/api/types.h>
#include <stdexcept>
#include <tuple>
#include <vector>

#define TUPLE_ITEM(tuple, index) std::get<index>(tuple)
#define TUPLE_ITEM_LAYER(tuple) TUPLE_ITEM(tuple, 0)
#define TUPLE_ITEM_PLUGIN_NAME(tuple) TUPLE_ITEM(tuple, 1)
#define TUPLE_ITEM_CONTENT_AFTER(tuple) TUPLE_ITEM(tuple, 2)
#define TUPLE_ITEM_CONTENT_BEFORE(tuple) TUPLE_ITEM(tuple, 3)

/**
 *  greatest_layer returns the plugin that has the greatest layer number
 */
size_t greatest_layer(const layer_informations &infos)
{
    auto max_element = std::max_element(
      infos.begin(), infos.end(),
      [](const layer_information &a, const layer_information &b) {
          return (TUPLE_ITEM_LAYER(a) < TUPLE_ITEM_LAYER(b));
      });
    return TUPLE_ITEM_LAYER(*max_element);
}

// plugin_layer &plugin_layer::operator=(plugin_layer &&plugin_layer)
// {
//     this->n = plugin_layer.n;
//     this->p = plugin_layer.p;

//     plugin_layer.n = 0;
//     plugin_layer.p = nullptr;

//     return *this;
// }

/**
 *  layer_informations {
 *      {1, "PrintAPI"}, {1, "CatchExceptionAPI"},
 *      {2, "APIReporter", {3, "AssemblyAPI"}
 *  };
 *
 * internally this will create an array of 3 elements
 * [PluginLayer *, PluginLayer* PluginLayer*] and plugins
 * that are with the same layer number we should add into the
 * this->p[layer_number] next node in the linked list
 *
 * [0] - nullptr
 * [1] - PluginLayer*{PRintAPI} -> PluginLayer{CatchExceptionAPI} ->nullptr
 * [2] - PluginLayer*{APIReporter} -> nullptr
 * [3] - PluginLayer*{AssemblyAPI} -> nullptr
 *
 *
 */
plugin_layer::plugin_layer(const layer_informations &&infos)
{
    // get the greatest layer in order to find
    // how many layers we need to create
    this->n = greatest_layer(infos);

    // create an array of n layers
    // because the highest layer will be at
    // this->p[this->n] and init all the block with 0
    this->p = new PluginLayer *[n + 1]();

    auto m = infos.size();
    auto already_done = std::vector<size_t>();

    for (size_t i = 0; i < m; i++) {
        // we will interate through all vector pairs
        // if the layer was already touched skip it
        auto idx = TUPLE_ITEM_LAYER(infos[i]);
        if (this->find_in(already_done, idx))
            continue;
        auto plugin_name = TUPLE_ITEM_PLUGIN_NAME(infos[i]);
        auto content_before = TUPLE_ITEM_CONTENT_BEFORE(infos[i]);
        auto content_after = TUPLE_ITEM_CONTENT_AFTER(infos[i]);

        // if not, create a new entry into our array
        this->p[idx] = new PluginLayer;
        this->p[idx]->data = new PluginReport;
        this->p[idx]->data->plugin_name = plugin_name;
        this->p[idx]->data->content_after = content_after;
        this->p[idx]->data->content_before = content_before;
        this->p[idx]->nextnode = nullptr;

        // append the rest of the plugin into our linked list
        this->append_into_linked_list(p[idx], idx, infos, idx);
        already_done.push_back(idx); // mark idx as done;
    }

    assert(this->p != nullptr);
    assert(this->p[n] != nullptr);
    assert(this->p[n]->data != nullptr);
    assert(this->p[n]->nextnode == nullptr);
}

plugin_layer::~plugin_layer()
{
    if ((!this->n) || (!this->p))
        return;

    for (size_t i = 0; i < this->n + 1; i++) {
        if (this->p[i] == nullptr)
            continue;

        do {
            assert(p[i]->data != nullptr);
            delete this->p[i]->data;
            this->p[i]->data = nullptr;

            auto next = this->p[i]->nextnode;
            delete this->p[i];
            this->p[i] = next;
        } while (this->p[i] != nullptr);
    }

    delete[] this->p;

    this->p = nullptr;
    this->n = 0;
}

bool plugin_layer::find_in(std::vector<size_t> arr, size_t item) const noexcept
{
    if (!arr.size())
        return false;

    return std::find(arr.begin(), arr.end(), item) != arr.end();
}

void plugin_layer::append_into_linked_list(PluginLayer *layer,
                                           size_t target_layer,
                                           const layer_informations &infos,
                                           size_t at) const noexcept
{
    size_t m = infos.size();
    for (size_t i = at + 1; i < m; i++) {
        // if the layer we are searching is not in our target layer
        // just skip it
        if (TUPLE_ITEM_LAYER(infos[i]) != target_layer)
            continue;

        // we found a plugin on the same layer and add it to the
        // current linked list
        layer->nextnode = new PluginLayer{0};
        layer->nextnode->data = new PluginReport{0};
        layer->nextnode->data->plugin_name = TUPLE_ITEM_PLUGIN_NAME(infos[i]);
        layer->nextnode->nextnode = nullptr;
        layer = layer->nextnode;
    }
}

PluginLayer **plugin_layer::get() const noexcept
{
    return this->p;
}

plugin_layer &plugin_layer::operator=(const plugin_layer &plugin_layer)
{
    if ((!plugin_layer.n) || (!plugin_layer.p))
        throw std::logic_error("invalid obj to copy ctor");

    // destroy what we have first if we have smth;
    if ((this->n) && (this->p))
        this->~plugin_layer();

    // alloc and copy the entire thing;
    // we don't need to call plugin_layer.dctor
    // because it's called automatically

    this->n = plugin_layer.n;

    this->p = new PluginLayer *[n + 1]();

    // iterate all the array of layers
    for (auto i = 0u; i < this->n + 1; i++) {
        // if we are at an empty layer, skip it
        if (plugin_layer.p[i] == nullptr)
            continue;

        // aloc a new layer;
        this->p[i] = new PluginLayer{0};

        auto to = this->p[i];
        auto from = plugin_layer.p[i];
        // alloc and copy the entire linked list;
        // element by element
        do {
            to->data = new PluginReport{0};
            // TODO(hoenir): should we alloc and copy them also?
            to->data->content_after = from->data->content_after;
            to->data->content_before = from->data->content_before;
            to->data->plugin_name = from->data->plugin_name;

            if (from->nextnode) {
                this->p[i]->nextnode = new PluginLayer{0};
                memcpy(to->nextnode, from->nextnode, sizeof(to->nextnode));
            }

            to = this->p[i]->nextnode;
            from = from->nextnode;
        } while (from != nullptr);
    }

    return *this;
}

plugin_layer &plugin_layer::operator=(plugin_layer &&plugin_layer)
{
    this->n = plugin_layer.n;
    this->p = plugin_layer.p;

    plugin_layer.n = 0;
    plugin_layer.p = nullptr;

    return *this;
}

plugin_layer::plugin_layer(const plugin_layer &plugin_layer)
{
    *this = plugin_layer;
}

plugin_layer::plugin_layer(plugin_layer &&plugin_layer)
{
    *this = std::move(plugin_layer);
}
