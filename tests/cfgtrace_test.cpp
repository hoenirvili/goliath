#include "cfgtrace.h"
#include "cfgtrace/api/types.h"
#include "cfgtrace/logger/logger.h"
#include "custom_params.h"
#include "fake_output_streamer.h"
#include "plugin_layer.h"
#include "plugin_report.h"
#include "virtual_memory.h"
#include "windows.h"
#include <catch2/catch.hpp>
#include <memory>
#include <vector>

TEST_CASE("The plugin is assumed to be run in layer 2", "[GetLayer]")
{
    size_t layer = GetLayer();
    REQUIRE(layer == PLUGIN_LAYER);
}

TEST_CASE("When the internal virtual memory is not initialised", "[DBTInit]")
{
    BOOL state = DBTInit();
    REQUIRE(state == FALSE);
}

TEST_CASE("When the internal virtual memory is initialised", "[DBTInit]")
{
    virtual_memory vm = virtual_memory();

    SECTION("file_mapping is now constructed but log name is not available")
    {
        BOOL state = DBTInit();
        REQUIRE(state == FALSE);
        auto it = vm.interation_count();
        REQUIRE(it == 0);
    }
}

TEST_CASE("When the internal virtual memory log is initialised", "[DBTInit]")
{
    virtual_memory vm = virtual_memory();
    vm.enable_log_name();
    fake_output_streamer fom = fake_output_streamer();
    bool is_set = logger::is_writer_set();
    REQUIRE(is_set == false);
    std::ostream *w = fom.writer();
    logger::set_writer(w);

    SECTION("file_mapping is constructed and log name is available")
    {
        BOOL state = DBTInit();
        REQUIRE(state == TRUE);
        fom.check("[CFGTrace] Init is called");
        fom.check("[CFGTrace] Init is called for iteration 1");
        auto it = vm.interation_count();
        REQUIRE(it == 1);
    }

    logger::unset_writer();
}

TEST_CASE("When the internal virtual memory has a log and is initialised", "[DBTInit]")
{
    virtual_memory vm = virtual_memory();
    vm.enable_log_name();
    fake_output_streamer fom = fake_output_streamer();
    bool is_set = logger::is_writer_set();
    REQUIRE(is_set == false);
    std::ostream *w = fom.writer();
    logger::set_writer(w);
    vm.set_iteration_count(1);
    SECTION("load empty control flow graph from memory")
    {
        BOOL state = DBTInit();
        REQUIRE(state == TRUE);
        fom.check("[CFGTrace] Init is called");
        fom.check("[CFGTrace] Init is called for iteration 2");
        auto it = vm.interation_count();
        REQUIRE(it == 2);
    }
    logger::unset_writer();
}

TEST_CASE("Append no branch instruction", "[DBTBeforeExecute]")
{
    auto cpb = std::make_unique<custom_params>(0x55232288, "mov eax, ebx", NO_BRANCH, 2, 0x55232288 + 2, 0);
    auto plb = std::make_unique<plugin_layer>(2, "PFGTrace");

    auto params = cpb->get();
    auto layers = plb->get();
    auto rptr = DBTBeforeExecute(params, layers);
    REQUIRE(rptr != nullptr);
    REQUIRE(rptr->content_after == nullptr);
    REQUIRE(rptr->content_before != nullptr);
    REQUIRE(rptr->plugin_name != nullptr);
    auto compare = !strcmp(rptr->plugin_name, "CFGTrace");
    REQUIRE(compare == true);
    auto report_ptr = plugin_report_ptr(rptr);
}

TEST_CASE("Append branch instruction", "[DBTBeforeExecute]")
{
    auto cpb =
      std::make_unique<custom_params>(0x55232288, "call 0x312512", CallType, 4, 0x55232288 + 150, 0x55232288 + 2);
    auto plb = std::make_unique<plugin_layer>(2, "OtherPlugin");

    auto params = cpb->get();
    auto layers = plb->get();
    auto rptr = DBTBeforeExecute(params, layers);
    REQUIRE(rptr != nullptr);
    REQUIRE(rptr->content_after == nullptr);
    REQUIRE(rptr->content_before != nullptr);
    REQUIRE(rptr->plugin_name != nullptr);
    auto compare = !strcmp(rptr->plugin_name, "CFGTrace");
    REQUIRE(compare == true);
    auto report_ptr = plugin_report_ptr(rptr);
}

using vector_of_params = std::vector<custom_params_ptr>;

TEST_CASE("Append multiple instructions", "[DBTBeforeExecute]")
{
    vector_of_params params;
    params.emplace_back(std::make_unique<custom_params>(0x10000000, "mov eax, ebx", NO_BRANCH, 2, 0x1000000 + 2, 0));
    params.emplace_back(std::make_unique<custom_params>(0x10000002, "mov eax, ebx", NO_BRANCH, 2, 0x55232288 + 150, 0));
    params.emplace_back(std::make_unique<custom_params>(0x10000004, "mov eax, ebx", NO_BRANCH, 2, 0x55232288 + 150, 0));
    params.emplace_back(std::make_unique<custom_params>(0x10000006, "mov eax, ebx", NO_BRANCH, 2, 0x55232288 + 150, 0));
    params.emplace_back(
      std::make_unique<custom_params>(0x10000008, "call 0x10000008", CallType, 4, 0x10000008 + 92, 0x10000008 + 2));
    params.emplace_back(
      std::make_unique<custom_params>(0x10000100, "add ecx, eax", NO_BRANCH, 2, 0x55232288 + 150, 0x55232288 + 2));
    params.emplace_back(std::make_unique<custom_params>(0x10000102, "ret", RetType, 2, 0, 0));
    params.emplace_back(
      std::make_unique<custom_params>(0x10000010, "mov eax, ebx", NO_BRANCH, 2, 0x55232288 + 150, 0x55232288 + 2));

    auto plb = std::make_unique<plugin_layer>(2, "OtherPlugin");
    auto layers = plb->get();

    for (const auto &param : params) {
        auto p = param->get();
        auto rptr = DBTBeforeExecute(p, layers);
        REQUIRE(rptr != nullptr);
        REQUIRE(rptr->content_after == nullptr);
        REQUIRE(rptr->content_before != nullptr);
        REQUIRE(rptr->plugin_name != nullptr);
        auto compare = !strcmp(rptr->plugin_name, "CFGTrace");
        REQUIRE(compare == true);
        auto report_ptr = plugin_report_ptr(rptr);
    }
}
