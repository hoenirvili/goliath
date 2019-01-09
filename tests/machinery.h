#pragma once

#include <goliath/api/types.h>

#include "test/custom_params.h"
#include "test/plugin_layer.h"

#include <functional>
#include <utility>
#include <vector>
#include <windows.h>

struct machinery {
    // define a pairs type that can hold a custom paramas
    // and a plugin layer used for constructing and passing
    // as parameters to the plugin api
    using pairs = std::pair<custom_params, plugin_layer>;

    // params list of parameters
    using params = std::vector<pairs>;

    using list_params_run = std::vector<custom_params>;

    using params_run = std::vector<list_params_run>;

    using batch_runs = std::vector<params>;

    using list_params = std::vector<custom_params>;

    // set a single layer for all the parameters
    plugin_layer single_layer;

    // single_layer_set is when we want to set the same layer
    // on every pair in our vector of params
    bool single_layer_set = false;

    // params a list of pair of parameters
    params list_of_params;

    batch_runs list_of_runs;

    void add_custom_params(const params_run &&runs);

    // add_single_layer add a single layer to the list of parameter pairs
    void add_single_layer(const layer_informations &&infos);
    // void add_custom_params(const params &&params);

    // before set of function;
    std::function<void()> run_before_dbtinit = nullptr;

    // after set of functions
    std::function<void(size_t)> run_after_dbtinit = nullptr;
    std::function<void(size_t)> run_after_dbtbefore = nullptr;
    std::function<void(size_t)> run_after_dbtbranch = nullptr;
    std::function<void(size_t)> run_after_dbtfinish = nullptr;

    // inspect
    std::function<void(BOOL, size_t)> inspect_init_state = nullptr;
    std::function<void(PluginReport *, size_t)> inspect_plugin_report = nullptr;
    std::function<void(PluginReport *, size_t)> inspect_finish_report = nullptr;

    void start();

private:
    using api_call = std::function<PluginReport *(void *, PluginLayer **)>;
    using api_calls_list = std::vector<api_call>;

    api_calls_list api_order_calls(const custom_params *cp);
};