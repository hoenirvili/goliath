#include "machinery.h"
#include "test/plugin_layer.h"

#include <catch2/catch.hpp>

#include <goliath.h>
#include <goliath/engine/engine.h>

#include <memory>
#include <stdexcept>
#include <vector>
#include <windows.h>

void machinery::add_custom_params(const machinery::params_run &&runs)
{
    if (!this->single_layer_set)
        throw std::logic_error("set single layer first");

    machinery::params params;
    for (const auto &list : runs) {
        for (const auto &el : list)
            params.emplace_back(el, this->single_layer);

        this->list_of_runs.emplace_back(params);
        params.clear();
    }
}

void machinery::add_single_layer(const layer_informations &&infos)
{
    this->single_layer_set = true;
    this->single_layer = plugin_layer(std::move(infos));
}

machinery::api_calls_list machinery::api_order_calls(const custom_params *cp)
{
    if (cp->branch())
        return api_calls_list{DBTBranching, DBTBeforeExecute};

    return api_calls_list{DBTBeforeExecute};
}

void machinery::start()
{
    if (this->list_of_runs.empty())
        throw std::logic_error("list_of_runs empty");

    for (const auto &params_list : this->list_of_runs) {
        if (this->run_before_dbtinit)
            this->run_before_dbtinit();

        // for every iteration DBTINIT should be called
        auto state = DBTInit();

        auto engine = engine::instance();
        auto pit = engine->cfg_iteration();
        auto it = *pit;

        if (this->inspect_init_state)
            this->inspect_init_state(state, it);

        if (this->run_after_dbtinit)
            this->run_after_dbtinit(it);

        // for every iteration we have a list of instructions;
        for (auto &params : params_list) {
            // get the list of instructions and layers
            auto layer = params.second.get();
            auto cp = params.first.get();

            // decide which api should we call and in which order
            auto api_calls = this->api_order_calls(&params.first);

            // call the apis in the order given
            for (const auto &api : api_calls) {
                auto report = api(cp, layer);

                if (this->inspect_plugin_report)
                    this->inspect_plugin_report(report, it);

                if (report)
                    delete report;
            }
        }

        auto report = DBTFinish();
        if (this->inspect_finish_report)
            this->inspect_finish_report(report, it);

        if (report)
            delete report;

        if (this->run_after_dbtfinish)
            this->run_after_dbtfinish(it);
    }
}