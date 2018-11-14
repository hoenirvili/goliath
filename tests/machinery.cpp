#include "machinery.h"
#include "test/plugin_layer.h"

#include <catch2/catch.hpp>

#include <cfgtrace.h>

#include <memory>
#include <stdexcept>
#include <vector>
#include <windows.h>

void machinery::add_single_layer(const layer_informations &&infos)
{
    this->single_layer_set = true;
    this->single_layer = plugin_layer(std::move(infos));
}

void machinery::add_custom_params(const std::vector<custom_params> &&params)
{
    if (!this->single_layer_set)
        throw std::logic_error("set the single layer first");

    auto n = params.size();
    for (auto i = 0u; i < n; i++) {
        this->list_of_params.emplace_back(
          std::make_pair(params[i], this->single_layer));
    }
}

machinery::api_calls_list machinery::api_order_calls(const custom_params *cp)
{
    if (cp->branch())
        return api_calls_list{DBTBranching, DBTBeforeExecute};

    return api_calls_list{DBTBeforeExecute};
}

void machinery::start()
{
    auto state = DBTInit();
    if (this->inspect_init_state)
        this->inspect_init_state(state);

    if (this->run_after_dbtinit)
        this->run_after_dbtinit();

    for (auto &params : this->list_of_params) {
        auto layer = params.second.get();
        auto cp = params.first.get();

        auto api_calls = this->api_order_calls(&params.first);

        for (const auto &api : api_calls) {
            auto report = api(cp, layer);

            if (this->inspect_plugin_report)
                this->inspect_plugin_report(report);

            if (report)
                delete report;
        }
    }

    auto report = DBTFinish();
    if (this->inspect_finish_report)
        this->inspect_finish_report(report);

    if (report)
        delete report;

    if (this->run_after_dbtfinish)
        this->run_after_dbtfinish();
}

// void machinery::add_custom_params(const machinery::params &&params)
// {
//     this->list_of_params = params;
// }